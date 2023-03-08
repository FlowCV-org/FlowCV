//
// Plugin DNN Text Detection
//

#include "dnn_text_detection.hpp"
#include <fstream>
#include <sstream>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

TextDetection::TextDetection() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Text_Detection");
    SetComponentCategory_(DSPatch::Category::Category_DNN);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 input
    SetInputCount_(1, {"in1"}, {IoType::Io_Type_CvMat});

    // 1 output
    SetOutputCount_(2, {"out", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // Set Defaults
    bbox_color_ = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    bbox_thickness_ = 2.0;
    scale_ = 1.0f / 255.0f;
    mean_[0] = 122.67891434;
    mean_[1] = 116.66876762;
    mean_[2] = 104.00698793;
    bin_thresh_ = 0.3f;
    poly_thresh_ = 0.5f;
    unclip_ratio_ = 2.0f;
    max_candidates_ = 200;
    model_res_[0] = 736;
    model_res_[1] = 736;
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

void TextDetection::InitDnn_()
{

    try {
        net_ = std::make_unique<cv::dnn::TextDetectionModel_DB>(model_path_);
        net_->setPreferableBackend(current_backend_);
        net_->setPreferableTarget(current_target_);
        net_->setBinaryThreshold(bin_thresh_).setPolygonThreshold(poly_thresh_).setUnclipRatio(unclip_ratio_).setMaxCandidates(max_candidates_);
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

void TextDetection::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    std::lock_guard<std::mutex> lk(io_mutex_);
    if (!in1->empty()) {
        if (IsEnabled() && !net_load_error && is_initialized_) {
            // Process Image
            cv::Mat orig, frame;
            in1->copyTo(orig);
            cv::Size modelSize = cv::Size(model_res_[0], model_res_[1]);
            std::vector<std::vector<cv::Point>> results;

            if (pre_proc_resize_) {
                if (model_res_[0] > 0 && model_res_[1] > 0) {
                    cv::resize(*in1, frame, modelSize);
                }
                else
                    frame = orig;
            }
            else
                frame = orig;

            try {
                net_->setBinaryThreshold(bin_thresh_).setPolygonThreshold(poly_thresh_).setUnclipRatio(unclip_ratio_).setMaxCandidates(max_candidates_);
                net_->setInputParams(scale_, modelSize, cv::Scalar(mean_[0], mean_[1], mean_[2]), swap_rb_, false);
                net_->detect(frame, results);
            }
            catch (std::exception &e) {
                std::cerr << GetInstanceName() << ", Error Computing DNN" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }

            nlohmann::json json_out;
            nlohmann::json detected;
            json_out["data_type"] = "objects";
            nlohmann::json ref;
            ref["w"] = orig.cols;
            ref["h"] = orig.rows;
            json_out["ref_frame"] = ref;

            for (const auto &res : results) {
                nlohmann::json jBbox;
                nlohmann::json cTmp;
                for (const auto &p : res) {
                    nlohmann::json jPoint;
                    if (pre_proc_resize_) {
                        float SX = (float)in1->cols / float(frame.cols);
                        float SY = (float)in1->rows / float(frame.rows);
                        jPoint["x"] = (int)((float)p.x * SX);
                        jPoint["y"] = (int)((float)p.y * SY);
                    }
                    else {
                        jPoint["x"] = p.x;
                        jPoint["y"] = p.y;
                    }
                    jBbox.emplace_back(jPoint);
                }
                cTmp["points"] = jBbox;
                detected.emplace_back(cTmp);
            }

            if (draw_detections_)
                polylines(frame, results, true, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255), bbox_thickness_);

            if (pre_proc_resize_) {
                cv::resize(frame, orig, cv::Size(in1->cols, in1->rows));
            }

            if (!detected.empty())
                json_out["data"] = detected;
            outputs.SetValue(1, json_out);
            outputs.SetValue(0, orig);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool TextDetection::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void TextDetection::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!model_path_.empty()) {
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
            if (ImGui::Combo(
                    CreateControlString("Backend", GetInstanceName()).c_str(), &dnn_backend_idx_,
                    [](void *data, int idx, const char **out_text) {
                        *out_text = ((const std::vector<std::pair<std::string, cv::dnn::Backend>> *)data)->at(idx).first.c_str();
                        return true;
                    },
                    (void *)&backend_list_, (int)backend_list_.size())) {
                // Update and get Target List
                current_backend_ = backend_list_.at(dnn_backend_idx_).second;
                dnn_backend_helper_.UpdateTargetList(backend_list_.at(dnn_backend_idx_).second);
                target_list_ = dnn_backend_helper_.GetTargetList();
                dnn_target_idx_ = 0;
                current_target_ = target_list_.at(0).second;
                needs_reinit_ = true;
            }
            ImGui::SetNextItemWidth(120);
            if (ImGui::Combo(
                    CreateControlString("Target", GetInstanceName()).c_str(), &dnn_target_idx_,
                    [](void *data, int idx, const char **out_text) {
                        *out_text = ((const std::vector<std::pair<std::string, cv::dnn::Target>> *)data)->at(idx).first.c_str();
                        return true;
                    },
                    (void *)&target_list_, (int)target_list_.size())) {
                current_target_ = target_list_.at(dnn_target_idx_).second;
                needs_reinit_ = true;
            }
        }
        if (!model_path_.empty()) {
            ImGui::Separator();
            if (ImGui::Button(CreateControlString("Clear Paths", GetInstanceName()).c_str())) {
                model_path_ = "";
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

        if (show_model_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Model", GetInstanceName()).c_str());

        if (model_dialog_.showFileDialog(CreateControlString("Set Model", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".caffemodel,.bin,.onnx,.pb,.pth,.weights,.t7,.net", &show_model_dialog_)) {
            model_path_ = model_dialog_.selected_path;
            show_model_dialog_ = false;
            needs_reinit_ = true;
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(70);
        ImGui::DragInt(CreateControlString("Max Candidates", GetInstanceName()).c_str(), &max_candidates_, 0.5f, 1, 500);
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
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Binary Threshold", GetInstanceName()).c_str(), &bin_thresh_, 0.01f, 0.0f, 1.0f, "%0.2f");
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Poly Threshold", GetInstanceName()).c_str(), &poly_thresh_, 0.01f, 0.0f, 1.0f, "%0.2f");
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Unclip Ratio", GetInstanceName()).c_str(), &unclip_ratio_, 0.01f, 0.0f, 5.0f, "%0.2f");
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("RGB", GetInstanceName()).c_str(), &swap_rb_);
        ImGui::Checkbox(CreateControlString("Preprocess Resize", GetInstanceName()).c_str(), &pre_proc_resize_);
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Detections", GetInstanceName()).c_str(), &draw_detections_);
        if (draw_detections_) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("BBox Thickness", GetInstanceName()).c_str(), &bbox_thickness_, 0.1f, 1, 10);
            ImGui::Separator();
            ImGui::ColorEdit3(CreateControlString("BBox Color", GetInstanceName()).c_str(), (float *)&bbox_color_);
        }
    }
}

std::string TextDetection::GetState()
{
    using namespace nlohmann;

    json state;

    state["model_path"] = model_path_;
    state["mean"] = {mean_[0], mean_[1], mean_[2]};
    state["scale"] = scale_;
    state["bin_thresh"] = bin_thresh_;
    state["poly_thresh"] = poly_thresh_;
    state["unclip_ratio"] = unclip_ratio_;
    state["max_candidates"] = max_candidates_;
    state["preproc"] = pre_proc_resize_;
    json jResize;
    jResize["w"] = model_res_[0];
    jResize["h"] = model_res_[1];
    state["model_res"] = jResize;
    if (draw_detections_) {
        state["bbox_thickness"] = bbox_thickness_;
        json tColor;
        tColor["R"] = bbox_color_.x;
        tColor["G"] = bbox_color_.y;
        tColor["B"] = bbox_color_.z;
        state["bbox_color"] = tColor;
    }
    state["swap_rb"] = swap_rb_;
    state["backend"] = (int)current_backend_;
    state["target"] = (int)current_target_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void TextDetection::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("model_path")) {
        if (!state["model_path"].empty())
            model_path_ = state["model_path"].get<std::string>();
    }

    if (state.contains("bbox_thickness"))
        bbox_thickness_ = state["bbox_thickness"].get<int>();

    if (state.contains("bbox_color")) {
        bbox_color_.x = state["bbox_color"]["R"].get<float>();
        bbox_color_.y = state["bbox_color"]["G"].get<float>();
        bbox_color_.z = state["bbox_color"]["B"].get<float>();
        draw_detections_ = true;
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

    if (state.contains("bin_thresh"))
        bin_thresh_ = state["bin_thresh"].get<float>();

    if (state.contains("poly_thresh"))
        poly_thresh_ = state["poly_thresh"].get<float>();

    if (state.contains("unclip_ratio"))
        unclip_ratio_ = state["unclip_ratio"].get<float>();

    if (state.contains("max_candidates"))
        max_candidates_ = state["max_candidates"].get<int>();

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
    if (!model_path_.empty()) {
        std::lock_guard<std::mutex> lk(io_mutex_);
        InitDnn_();
    }
}

}  // End Namespace DSPatch::DSPatchables
