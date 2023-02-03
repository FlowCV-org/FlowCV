//
// Plugin DNN Text Recognition
//

#include "dnn_text_recognition.hpp"
#include <fstream>
#include <sstream>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

void fourPointsTransform(const cv::Mat& frame, const std::vector<cv::Point2f> &vertices, cv::Mat& result)
{
    const cv::Size outputSize = cv::Size(100, 32);

    cv::Point2f targetVertices[4] = {
        cv::Point(0, outputSize.height - 1),
        cv::Point(0, 0),
        cv::Point(outputSize.width - 1, 0),
        cv::Point(outputSize.width - 1, outputSize.height - 1)
    };
    cv::Mat rotationMatrix = getPerspectiveTransform(vertices.data(), targetVertices);

    warpPerspective(frame, result, rotationMatrix, outputSize);
}

namespace DSPatch::DSPatchables
{

TextRecognition::TextRecognition()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Text_Recognition");
    SetComponentCategory_(DSPatch::Category::Category_DNN);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 input
    SetInputCount_( 2, {"in1", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    // 1 output
    SetOutputCount_( 2, {"out", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    // Set Defaults
    text_pos_ = cv::Point2i(0, 0);
    text_scale_ = 0.5;
    text_color_ = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    text_thickness_ = 2.0;
    scale_ = 1.0f / 127.5;
    mean_[0] = 127.5;
    mean_[1] = 127.5;
    mean_[2] = 127.5;
    model_res_[0] = 100;
    model_res_[1] = 32;
    swap_rb_ = true;
    net_load_error = false;
    dnn_backend_idx_ = 0;
    current_backend_ = cv::dnn::DNN_BACKEND_DEFAULT;
    dnn_target_idx_ = 0;
    current_target_ = cv::dnn::DNN_TARGET_CPU;
    backend_list_ = dnn_backend_helper_.GetBackEndList();
    dnn_backend_helper_.UpdateTargetList((cv::dnn::Backend)dnn_backend_idx_);
    target_list_ = dnn_backend_helper_.GetTargetList();

    SetEnabled(true);

}

void TextRecognition::InitDnn_()
{
    // Load Vocabulary list
    vocabulary_.clear();
    if (!voc_path_.empty()) {
        std::ifstream ifs(voc_path_);
        if (!ifs.is_open()) {
            std::cerr << GetInstanceName() << ", File " + voc_path_ + " not found" << std::endl;
            return;
        }
        else {
            std::string line;
            while (std::getline(ifs, line)) {
                vocabulary_.emplace_back(line);
            }
        }
    }

    try {
        net_ = std::make_unique<cv::dnn::TextRecognitionModel>(model_path_);
        net_->setPreferableBackend(current_backend_);
        net_->setPreferableTarget(current_target_);
        net_->setVocabulary(vocabulary_);
        net_->setDecodeType("CTC-greedy");
    }
    catch (...) {
        std::cerr << "DNN Initialization Error" << std::endl;
        net_load_error = true;
        is_initialized_ = false;
        return;
    }
    net_load_error = false;
    is_initialized_ = true;
    needs_reinit_ = false;
}

void TextRecognition::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in_json = inputs.GetValue<nlohmann::json>( 1 );
    if ( !in1 ) {
        return;
    }

    std::lock_guard<std::mutex> lk(io_mutex_);
    if (!in1->empty()) {
        if (IsEnabled() && !net_load_error && is_initialized_) {
            // Process Image
            cv::Mat orig, frame;
            in1->copyTo(orig);
            cv::Size modelSize = cv::Size(model_res_[0], model_res_[1]);

            if (pre_proc_resize_) {
                if (model_res_[0] > 0 && model_res_[1] > 0) {
                    cv::resize(*in1, frame, modelSize);
                }
                else
                    frame = orig;
            }
            else
                frame = orig;

            nlohmann::json json_out;
            nlohmann::json detected;
            json_out["data_type"] = "text";
            nlohmann::json ref;
            ref["w"] = orig.cols;
            ref["h"] = orig.rows;
            json_out["ref_frame"] = ref;

            try {
                net_->setInputParams(scale_, modelSize, cv::Scalar(mean_[0], mean_[1], mean_[2]));
                cv::Mat recInput = frame;
                if (in_json) {
                    nlohmann::json json_data = *in_json;
                    if (!json_data.empty()) {
                        bool hasObjects = false;
                        if (json_data.contains("data_type")) {
                            if (json_data["data_type"].get<std::string>() == "objects") {
                                if (json_data.contains("data")) {
                                    if (!json_data["data"].empty())
                                        hasObjects = true;
                                }
                            }
                        }
                        if (hasObjects) {
                            for (const auto &d : json_data["data"]) {
                                if (d.contains("points")) {
                                    std::vector<cv::Point2f> quadrangle_2f;
                                    for (const auto &bb : d["points"]) {
                                        cv::Point2f pt;
                                        pt.x = bb["x"].get<float>();
                                        pt.y = bb["y"].get<float>();
                                        quadrangle_2f.emplace_back(pt);
                                    }
                                    cv::Mat cropped;
                                    fourPointsTransform(recInput, quadrangle_2f, cropped);
                                    net_->setInputParams(scale_, modelSize, cv::Scalar(mean_[0], mean_[1], mean_[2]), swap_rb_, false);
                                    std::string recognitionResult = net_->recognize(cropped);
                                    nlohmann::json cTmp;
                                    nlohmann::json jBbox;
                                    cv::Rect bbox = cv::boundingRect(quadrangle_2f);
                                    jBbox["x"] = bbox.x;
                                    jBbox["y"] = bbox.y;
                                    jBbox["w"] = bbox.width;
                                    jBbox["h"] = bbox.height;
                                    cTmp["bbox"] = jBbox;
                                    cTmp["text"] = recognitionResult;
                                    detected.emplace_back(cTmp);
                                    if (draw_text_) {
                                        cv::putText(frame, recognitionResult, cv::Point((int)quadrangle_2f.at(0).x + text_pos_.x, (int)quadrangle_2f.at(0).y + text_pos_.y),
                                                    cv::FONT_HERSHEY_SIMPLEX,
                                                    text_scale_,
                                                    cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (std::exception &e) {
                std::cerr << GetInstanceName() << ", Error Computing DNN" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }

            if (pre_proc_resize_) {
                cv::resize(frame, orig, cv::Size(in1->cols, in1->rows));
            }

            if (!detected.empty())
                json_out["data"] = detected;
            outputs.SetValue(1, json_out);
            outputs.SetValue(0, orig);
        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool TextRecognition::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void TextRecognition::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!model_path_.empty() && !voc_path_.empty()) {
            std::string button_str;
            if (net_load_error)
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error Initializing Network");
            if (needs_reinit_ && is_initialized_)
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Network Needs To Be Reinitialized!");

            if (is_initialized_)
                button_str = "Reinitialize Network";
            else {
                button_str = "Initialize Network";
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Network Needs To Be Initialized!");
            }
            if (ImGui::Button(CreateControlString(button_str.c_str(), GetInstanceName()).c_str())) {
                std::lock_guard<std::mutex> lk(io_mutex_);
                InitDnn_();
            }
            ImGui::SetNextItemWidth(120);
            if (ImGui::Combo(CreateControlString("Backend", GetInstanceName()).c_str(),
                             &dnn_backend_idx_, [](void* data, int idx, const char** out_text) {
                    *out_text = ((const std::vector<std::pair<std::string, cv::dnn::Backend>>*)data)->at(idx).first.c_str();
                    return true;
                }, (void*)&backend_list_, (int)backend_list_.size())) {
                // Update and get Target List
                current_backend_ = backend_list_.at(dnn_backend_idx_).second;
                dnn_backend_helper_.UpdateTargetList(backend_list_.at(dnn_backend_idx_).second);
                target_list_ = dnn_backend_helper_.GetTargetList();
                dnn_target_idx_ = 0;
                current_target_ = target_list_.at(0).second;
                needs_reinit_ = true;
            }
            ImGui::SetNextItemWidth(120);
            if (ImGui::Combo(CreateControlString("Target", GetInstanceName()).c_str(),
                             &dnn_target_idx_, [](void* data, int idx, const char** out_text) {
                    *out_text = ((const std::vector<std::pair<std::string, cv::dnn::Target>>*)data)->at(idx).first.c_str();
                    return true;
                }, (void*)&target_list_, (int)target_list_.size())) {
                current_target_ = target_list_.at(dnn_target_idx_).second;
                needs_reinit_ = true;
            }
        }
        if (!model_path_.empty()) {
            ImGui::Separator();
            if (ImGui::Button(CreateControlString("Clear Paths", GetInstanceName()).c_str())) {
                model_path_ = "";
                voc_path_ = "";
                needs_reinit_ = true;
            }
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Set Model File", GetInstanceName()).c_str())) {
            show_model_dialog_ = true;
        }
        ImGui::Text("Model File:");
        if (model_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", model_path_.c_str());

        if(show_model_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Model", GetInstanceName()).c_str());

        if(model_dialog_.showFileDialog(CreateControlString("Set Model", GetInstanceName()),
                                        imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                                        ImVec2(700, 310), ".caffemodel,.bin,.onnx,.pb,.pth,.weights,.t7,.net",
                                        &show_model_dialog_))
        {
            model_path_ = model_dialog_.selected_path;
            show_model_dialog_ = false;
            needs_reinit_ = true;
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Set Vocabulary File", GetInstanceName()).c_str())) {
            show_voc_dialog_ = true;
        }
        ImGui::Text("Vocabulary File:");
        if (voc_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", voc_path_.c_str());

        if(show_voc_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Vocabulary", GetInstanceName()).c_str());

        if(vocab_dialog_.showFileDialog(CreateControlString("Set Vocabulary", GetInstanceName()),
                                         imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                                         ImVec2(700, 310), ".txt",
                                         &show_voc_dialog_))
        {
            voc_path_ = vocab_dialog_.selected_path;
            show_voc_dialog_ = false;
            needs_reinit_ = true;
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(180);
        ImGui::DragFloat3(CreateControlString("Mean", GetInstanceName()).c_str(), mean_, 0.1f, 0.0f, 500.0f, "%0.2f");
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &scale_, 0.0001f, 0.0f, 10.0f, "%0.7f");
        ImGui::SetNextItemWidth(70);
        if (ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &model_res_[0], 0.5f, 224, 1000)) {
            needs_reinit_ = true;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70);
        if (ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &model_res_[1], 0.5f, 224, 1000)) {
            needs_reinit_ = true;
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("RGB", GetInstanceName()).c_str(), &swap_rb_);
        ImGui::Checkbox(CreateControlString("Preprocess Resize", GetInstanceName()).c_str(), &pre_proc_resize_);
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Detections", GetInstanceName()).c_str(), &draw_text_);
        if (draw_text_) {
            ImGui::Text("Text Offset:");
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("X", GetInstanceName()).c_str(), &text_pos_.x, 0.5f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Y", GetInstanceName()).c_str(), &text_pos_.y, 0.5f);
            ImGui::Separator();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Text Scale", GetInstanceName()).c_str(), &text_scale_, 0.01f, 0.0f, 100.0f);
            ImGui::Separator();
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Text Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.1f, 1, 10);
            ImGui::Separator();
            ImGui::ColorEdit3(CreateControlString("Text Color", GetInstanceName()).c_str(), (float *) &text_color_);
        }
    }

}

std::string TextRecognition::GetState()
{
    using namespace nlohmann;

    json state;

    state["model_path"] = model_path_;
    state["voc_path"] = voc_path_;
    state["mean"] = {mean_[0], mean_[1], mean_[2]};
    state["scale"] = scale_;
    state["preproc"] = pre_proc_resize_;
    json jResize;
    jResize["w"] = model_res_[0];
    jResize["h"] = model_res_[1];
    state["model_res"] = jResize;
    if (draw_text_) {
        json pos;
        pos["x"] = text_pos_.x;
        pos["y"] = text_pos_.y;
        state["txt_position"] = pos;
        state["txt_scale"] = text_scale_;
        state["txt_thickness"] = text_thickness_;
        json tColor;
        tColor["R"] = text_color_.x;
        tColor["G"] = text_color_.y;
        tColor["B"] = text_color_.z;
        state["txt_color"] = tColor;
    }
    state["swap_rb"] = swap_rb_;
    state["backend"] = (int)current_backend_;
    state["target"] = (int)current_target_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void TextRecognition::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("model_path")) {
        if (!state["model_path"].empty())
            model_path_ = state["model_path"].get<std::string>();
    }

    if (state.contains("voc_path")) {
        if (!state["voc_path"].empty())
            voc_path_ = state["voc_path"].get<std::string>();
    }

    if (state.contains("txt_position")) {
        text_pos_.x = state["txt_position"]["x"].get<int>();
        text_pos_.y = state["txt_position"]["y"].get<int>();
    }
    if (state.contains("txt_scale"))
        text_scale_ = state["txt_scale"].get<float>();

    if (state.contains("txt_thickness"))
        text_thickness_ = state["txt_thickness"].get<int>();

    if (state.contains("txt_color")) {
        text_color_.x = state["txt_color"]["R"].get<float>();
        text_color_.y = state["txt_color"]["G"].get<float>();
        text_color_.z = state["txt_color"]["B"].get<float>();
        draw_text_ = true;
    }

    if (state.contains("mean")) {
        mean_[0] = state["mean"][0].get<float>();
        mean_[1] = state["mean"][1].get<float>();
        mean_[2] = state["mean"][2].get<float>();
    }

    if (state.contains("model_res")) {
        model_res_[0] = state["model_res"]["w"].get<int>();
        model_res_[1] = state["model_res"]["h"].get<int>();
    }

    if (state.contains("preproc"))
        pre_proc_resize_ = state["preproc"].get<bool>();

    if (state.contains("scale"))
        scale_ = state["scale"].get<float>();

    if (state.contains("swap_rb"))
        swap_rb_ = state["swap_rb"].get<bool>();

    if (state.contains("backend"))
        current_backend_ = (cv::dnn::Backend)state["backend"].get<int>();

    if (state.contains("target"))
        current_target_ = (cv::dnn::Target)state["target"].get<int>();

    // Check Backend and Target
    backend_list_ = dnn_backend_helper_.GetBackEndList();
    bool backendMatch = false;
    for (int i = 0; i < backend_list_.size(); i++) {
        if (backend_list_[i].second == current_backend_) {
            backendMatch = true;
            dnn_backend_idx_ = i;
            bool targetMatch = false;
            dnn_backend_helper_.UpdateTargetList(backend_list_[i].second);
            target_list_ = dnn_backend_helper_.GetTargetList();
            for (int j = 0; j < target_list_.size(); j++) {
                if (target_list_[j].second == current_target_) {
                    targetMatch = true;
                    dnn_target_idx_ = j;
                    break;
                }
            }
            if (!targetMatch)
                current_target_ = cv::dnn::DNN_TARGET_CPU;
            break;
        }
    }
    if (!backendMatch)
        current_backend_ = cv::dnn::DNN_BACKEND_DEFAULT;

    // Check paths
    if (!model_path_.empty() && !voc_path_.empty()) {
        std::lock_guard<std::mutex> lk(io_mutex_);
        InitDnn_();
    }

}

} // End Namespace DSPatch::DSPatchables
