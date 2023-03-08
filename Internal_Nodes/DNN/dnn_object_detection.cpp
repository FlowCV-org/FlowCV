//
// Plugin DNN Object Detection
//

#include "dnn_object_detection.hpp"
#include <fstream>
#include <sstream>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

ObjectDetection::ObjectDetection() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Object_Detection");
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
    text_pos_ = cv::Point2i(10, 40);
    text_scale_ = 0.5;
    text_color_ = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    text_thickness_ = 2.0;
    scale_ = 1.0f;
    conf_thresh_ = 0.5f;
    nms_thresh_ = 0.4f;
    model_res_[0] = 416;
    model_res_[1] = 416;
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

void ObjectDetection::InitDnn_()
{
    // Load Classes list
    class_list_.clear();
    if (!classes_path_.empty()) {
        std::ifstream ifs(classes_path_);
        if (!ifs.is_open()) {
            std::cerr << GetInstanceName() << ", File " + classes_path_ + " not found" << std::endl;
        }
        else {
            std::string line;
            while (std::getline(ifs, line)) {
                class_list_.emplace_back(line);
            }
        }
    }

    try {
        net_ = std::make_unique<cv::dnn::Net>(cv::dnn::readNet(model_path_, config_path_, ""));
        net_->setPreferableBackend(current_backend_);
        net_->setPreferableTarget(current_target_);
        out_names_ = net_->getUnconnectedOutLayersNames();
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

void ObjectDetection::DrawPredictions_(int classId, float conf, cv::Rect box, cv::Mat &frame)
{
    std::string label;
    if (!class_list_.empty()) {
        if (classId < (int)class_list_.size())
            label = class_list_.at(classId) + ": " + cv::format("%.2f", conf);
        else
            return;
    }
    else {
        label = cv::format("%.2f", conf);
    }

    rectangle(frame, cv::Point(box.x, box.y), cv::Point(box.x + box.width, box.y + box.height),
        cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);

    int baseLine;
    cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, text_scale_, text_thickness_, &baseLine);

    box.y = cv::max(box.y, labelSize.height);
    rectangle(frame, cv::Point(box.x + text_pos_.x, (box.y - labelSize.height) + text_pos_.y),
        cv::Point((box.x + labelSize.width) + text_pos_.x, (box.y + baseLine) + text_pos_.y), cv::Scalar::all(64), cv::FILLED);
    putText(frame, label, cv::Point(box.x + text_pos_.x, box.y + text_pos_.y), cv::FONT_HERSHEY_SIMPLEX, text_scale_,
        cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
}

void ObjectDetection::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    std::lock_guard<std::mutex> lk(io_mutex_);
    if (!in1->empty()) {
        if (IsEnabled() && !net_load_error && is_initialized_) {
            // Process Image
            cv::Mat orig, frame, blob, prob;
            in1->copyTo(orig);
            cv::Size modelSize = cv::Size(model_res_[0], model_res_[1]);
            std::vector<cv::Mat> outs;

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
                cv::dnn::blobFromImage(frame, blob, 1.0, modelSize, cv::Scalar(), swap_rb_, crop_, CV_8U);
                net_->setInput(blob, "", scale_, cv::Scalar(mean_[0], mean_[1], mean_[2]));
                if (net_->getLayer(0)->outputNameToIndex("im_info") != -1) {  // Faster-RCNN or R-FCN
                    cv::resize(frame, frame, modelSize);
                    cv::Mat imInfo = (cv::Mat_<float>(1, 3) << modelSize.height, modelSize.width, 1.6f);
                    net_->setInput(imInfo, "im_info");
                }

                net_->forward(outs, out_names_);
            }
            catch (std::exception &e) {
                std::cerr << GetInstanceName() << ", Error Computing DNN" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }

            std::vector<int> outLayers = net_->getUnconnectedOutLayers();
            std::string outLayerType = net_->getLayer(outLayers[0])->type;
            std::vector<int> classIds;
            std::vector<float> confidences;
            std::vector<cv::Rect> boxes;

            if (outLayerType == "Region") {
                for (auto &out : outs) {
                    auto data = (float *)out.data;
                    for (int j = 0; j < out.rows; ++j, data += out.cols) {
                        cv::Mat scores = out.row(j).colRange(5, out.cols);
                        cv::Point classIdPoint;
                        double confidence;
                        minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
                        if (confidence > conf_thresh_) {
                            int centerX = (int)(data[0] * (float)orig.cols);
                            int centerY = (int)(data[1] * (float)orig.rows);
                            int width = (int)(data[2] * (float)orig.cols);
                            int height = (int)(data[3] * (float)orig.rows);
                            int left = centerX - width / 2;
                            int top = centerY - height / 2;

                            classIds.emplace_back(classIdPoint.x);
                            confidences.emplace_back((float)confidence);
                            boxes.emplace_back(left, top, width, height);
                        }
                    }
                }
            }
            else if (outLayerType == "DetectionOutput") {
                for (auto &out : outs) {
                    auto *data = (float *)out.data;
                    for (size_t i = 0; i < out.total(); i += 7) {
                        float confidence = data[i + 2];
                        if (confidence > conf_thresh_) {
                            int left = (int)data[i + 3];
                            int top = (int)data[i + 4];
                            int right = (int)data[i + 5];
                            int bottom = (int)data[i + 6];
                            int width = right - left + 1;
                            int height = bottom - top + 1;
                            if (width <= 2 || height <= 2) {
                                left = (int)(data[i + 3] * (float)orig.cols);
                                top = (int)(data[i + 4] * (float)orig.rows);
                                right = (int)(data[i + 5] * (float)orig.cols);
                                bottom = (int)(data[i + 6] * (float)orig.rows);
                                width = right - left + 1;
                                height = bottom - top + 1;
                            }
                            if (data[i + 1] > 0) {
                                classIds.emplace_back((int)(data[i + 1]) - 1);  // Skip 0th background class id.
                                boxes.emplace_back(left, top, width, height);
                                confidences.emplace_back(confidence);
                            }
                        }
                    }
                }
            }
            else {
                std::cout << GetInstanceName() << ", Unknown output layer type: " << outLayerType << std::endl;
                return;
            }

            nlohmann::json json_out;
            nlohmann::json detected;
            json_out["data_type"] = "objects";
            nlohmann::json ref;
            ref["w"] = orig.cols;
            ref["h"] = orig.rows;
            json_out["ref_frame"] = ref;

            if (outLayers.size() > 1 || outLayerType == "Region") {
                std::vector<int> indices;
                cv::dnn::NMSBoxes(boxes, confidences, conf_thresh_, nms_thresh_, indices);
                for (int idx : indices) {
                    cv::Rect box = boxes[idx];
                    nlohmann::json cTmp;
                    if (!class_list_.empty())
                        if (classIds[idx] < class_list_.size())
                            cTmp["class"] = class_list_.at(classIds[idx]);
                    cTmp["conf"] = confidences[idx];
                    nlohmann::json jBbox;
                    jBbox["x"] = box.x;
                    jBbox["y"] = box.y;
                    jBbox["w"] = box.width;
                    jBbox["h"] = box.height;
                    cTmp["bbox"] = jBbox;
                    detected.emplace_back(cTmp);
                    if (draw_class_)
                        DrawPredictions_(classIds[idx], confidences[idx], box, orig);
                }
            }
            else {
                for (size_t idx = 0; idx < boxes.size(); ++idx) {
                    cv::Rect box = boxes[idx];
                    nlohmann::json cTmp;
                    if (!class_list_.empty())
                        if (classIds[idx] < class_list_.size())
                            cTmp["class"] = class_list_.at(classIds[idx]);
                    cTmp["conf"] = confidences[idx];
                    nlohmann::json jBbox;
                    jBbox["x"] = box.x;
                    jBbox["y"] = box.y;
                    jBbox["w"] = box.width;
                    jBbox["h"] = box.height;
                    cTmp["bbox"] = jBbox;
                    detected.emplace_back(cTmp);
                    if (draw_class_)
                        DrawPredictions_(classIds[idx], confidences[idx], box, orig);
                }
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

bool ObjectDetection::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ObjectDetection::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!model_path_.empty() && !config_path_.empty()) {
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
        if (!model_path_.empty() || !config_path_.empty() || !classes_path_.empty()) {
            ImGui::Separator();
            if (ImGui::Button(CreateControlString("Clear Paths", GetInstanceName()).c_str())) {
                model_path_ = "";
                config_path_ = "";
                classes_path_ = "";
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
        if (ImGui::Button(CreateControlString("Set Config File", GetInstanceName()).c_str())) {
            show_config_dialog_ = true;
        }
        ImGui::Text("Config File:");
        if (config_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", config_path_.c_str());

        if (show_config_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Config", GetInstanceName()).c_str());

        if (config_dialog_.showFileDialog(CreateControlString("Set Config", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".prototxt,.txt,.pbtxt,.yml,.cfg,.xml", &show_config_dialog_)) {
            config_path_ = config_dialog_.selected_path;
            show_config_dialog_ = false;
            needs_reinit_ = true;
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Set Classes File", GetInstanceName()).c_str())) {
            show_classes_dialog_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(CreateControlString("Clear Classes File", GetInstanceName()).c_str())) {
            classes_path_ = "";
            needs_reinit_ = true;
        }
        ImGui::Text("Classes File:");
        if (classes_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", classes_path_.c_str());

        if (show_classes_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Classes", GetInstanceName()).c_str());

        if (classes_dialog_.showFileDialog(CreateControlString("Set Classes", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".txt", &show_classes_dialog_)) {
            classes_path_ = classes_dialog_.selected_path;
            show_classes_dialog_ = false;
            needs_reinit_ = true;
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(180);
        ImGui::DragFloat3(CreateControlString("Mean", GetInstanceName()).c_str(), mean_, 0.1f, 0.0f, 500.0f, "%0.2f");
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &scale_, 0.0001f, 0.0f, 10.0f, "%0.7f");
        ImGui::SetNextItemWidth(70);
        ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &model_res_[0], 0.5f, 224, 1000);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70);
        ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &model_res_[1], 0.5f, 224, 1000);
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Conf. Threshold", GetInstanceName()).c_str(), &conf_thresh_, 0.01f, 0.0f, 1.0f, "%0.2f");
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("NMS Threshold", GetInstanceName()).c_str(), &nms_thresh_, 0.01f, 0.0f, 1.0f, "%0.2f");
        ImGui::Checkbox(CreateControlString("RGB", GetInstanceName()).c_str(), &swap_rb_);
        if (ImGui::Checkbox(CreateControlString("Preprocess Resize", GetInstanceName()).c_str(), &pre_proc_resize_)) {
            if (!pre_proc_resize_)
                crop_ = false;
        }
        if (pre_proc_resize_)
            ImGui::Checkbox(CreateControlString("Crop", GetInstanceName()).c_str(), &crop_);
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Class", GetInstanceName()).c_str(), &draw_class_);
        if (draw_class_) {
            ImGui::Text("Text Position:");
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
            ImGui::ColorEdit3(CreateControlString("Text Color", GetInstanceName()).c_str(), (float *)&text_color_);
        }
    }
}

std::string ObjectDetection::GetState()
{
    using namespace nlohmann;

    json state;

    state["model_path"] = model_path_;
    state["config_path"] = config_path_;
    state["classes_path"] = classes_path_;
    state["mean"] = {mean_[0], mean_[1], mean_[2]};
    state["scale"] = scale_;
    state["crop"] = crop_;
    state["conf_thresh"] = conf_thresh_;
    state["preproc"] = pre_proc_resize_;
    json jResize;
    jResize["w"] = model_res_[0];
    jResize["h"] = model_res_[1];
    state["model_res"] = jResize;
    if (draw_class_) {
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

void ObjectDetection::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    draw_class_ = false;

    if (state.contains("model_path")) {
        if (!state["model_path"].empty())
            model_path_ = state["model_path"].get<std::string>();
    }
    if (state.contains("config_path")) {
        if (!state["config_path"].empty())
            config_path_ = state["config_path"].get<std::string>();
    }
    if (state.contains("classes_path")) {
        if (!state["classes_path"].empty())
            classes_path_ = state["classes_path"].get<std::string>();
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
        draw_class_ = true;
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

    if (state.contains("conf_thresh"))
        conf_thresh_ = state["conf_thresh"].get<float>();

    if (state.contains("swap_rb"))
        swap_rb_ = state["swap_rb"].get<bool>();

    if (state.contains("crop"))
        crop_ = state["crop"].get<bool>();

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
    if (!model_path_.empty() && !config_path_.empty()) {
        std::lock_guard<std::mutex> lk(io_mutex_);
        InitDnn_();
    }
}

}  // End Namespace DSPatch::DSPatchables
