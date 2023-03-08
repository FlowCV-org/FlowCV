//
// Plugin DNN Classification
//

#include "dnn_segmentation.hpp"
#include <fstream>
#include <sstream>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Segmentation::Segmentation() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Segmentation");
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
    scale_ = 1.0f;
    model_res_[0] = 224;
    model_res_[1] = 224;
    swap_rb_ = false;
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

void Segmentation::InitDnn_()
{
    // Load Classes list
    class_list_.clear();
    std::ifstream ifsClasses(classes_path_);
    if (!ifsClasses.is_open()) {
        std::cerr << GetInstanceName() << ", File " + classes_path_ + " not found" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(ifsClasses, line)) {
        class_list_.emplace_back(line);
    }

    // Open file with colors.
    colors_list_.clear();
    if (!colors_path_.empty()) {
        std::ifstream ifsColors(colors_path_);
        if (!ifsColors.is_open()) {
            std::cerr << GetInstanceName() << ", File " + colors_path_ + " not found" << std::endl;
            return;
        }
        while (std::getline(ifsColors, line)) {
            std::stringstream colorStr(line);
            cv::Vec3b color;
            std::string item;
            int i = 0;
            while (getline(colorStr, item, ' ')) {
                color[i] = std::stoi(item);
                i++;
                if (i > 2)
                    break;
            }
            std::cout << std::endl;
            colors_list_.push_back(color);
        }
    }

    try {
        if (!config_path_.empty())
            net_ = std::make_unique<cv::dnn::Net>(cv::dnn::readNet(model_path_, config_path_, ""));
        else
            net_ = std::make_unique<cv::dnn::Net>(cv::dnn::readNet(model_path_, "", ""));
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

void Segmentation::ColorizeSegmentation_(const cv::Mat &score, cv::Mat &segm)
{
    const int rows = score.size[2];
    const int cols = score.size[3];
    const int chns = score.size[1];

    if (colors_list_.empty()) {
        // Generate colors.
        colors_list_.emplace_back();
        for (int i = 1; i < chns; ++i) {
            cv::Vec3b color;
            for (int j = 0; j < 3; ++j)
                color[j] = (colors_list_[i - 1][j] + rand() % 256) / 2;
            colors_list_.emplace_back(color);
        }
    }
    else if (chns != (int)colors_list_.size()) {
        std::cerr << cv::format("Number of output classes doesn't match number of colors (%d != %zu)", chns, colors_list_.size()) << std::endl;
        return;
    }

    cv::Mat maxCl = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::Mat maxVal(rows, cols, CV_32FC1, score.data);
    for (int ch = 1; ch < chns; ch++) {
        for (int row = 0; row < rows; row++) {
            const auto *ptrScore = score.ptr<float>(0, ch, row);
            auto *ptrMaxCl = maxCl.ptr<uint8_t>(row);
            auto *ptrMaxVal = maxVal.ptr<float>(row);
            for (int col = 0; col < cols; col++) {
                if (ptrScore[col] > ptrMaxVal[col]) {
                    ptrMaxVal[col] = ptrScore[col];
                    ptrMaxCl[col] = (uchar)ch;
                }
            }
        }
    }

    segm.create(rows, cols, CV_8UC3);
    for (int row = 0; row < rows; row++) {
        const uchar *ptrMaxCl = maxCl.ptr<uchar>(row);
        auto *ptrSegm = segm.ptr<cv::Vec3b>(row);
        for (int col = 0; col < cols; col++) {
            ptrSegm[col] = colors_list_[ptrMaxCl[col]];
        }
    }
}

void Segmentation::Process_(SignalBus const &inputs, SignalBus &outputs)
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
            json_out["data_type"] = "segmentation";
            nlohmann::json ref;
            ref["w"] = orig.cols;
            ref["h"] = orig.rows;
            json_out["ref_frame"] = ref;

            try {
                cv::dnn::blobFromImage(frame, blob, scale_, modelSize, cv::Scalar(mean_[0], mean_[1], mean_[2]), swap_rb_, false);
                net_->setInput(blob);
                cv::Mat score = net_->forward();
                cv::Mat segm;
                ColorizeSegmentation_(score, segm);
                resize(segm, frame, orig.size(), 0, 0, cv::INTER_NEAREST);
            }
            catch (std::exception &e) {
                std::cerr << GetInstanceName() << ", Error Computing DNN" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }
            if (colors_list_.size() == class_list_.size()) {
                for (int i = 0; i < colors_list_.size(); i++) {
                    nlohmann::json cTmp;
                    cTmp["name"] = class_list_.at(i);
                    nlohmann::json jColor;
                    jColor["b"] = colors_list_.at(i)[0];
                    jColor["g"] = colors_list_.at(i)[1];
                    jColor["r"] = colors_list_.at(i)[2];
                    cTmp["color"] = jColor;
                    detected.emplace_back(cTmp);
                }
            }

            if (!detected.empty())
                json_out["data"] = detected;
            outputs.SetValue(1, json_out);
            outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Segmentation::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Segmentation::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!model_path_.empty() && !classes_path_.empty()) {
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
                colors_path_ = "";
                needs_reinit_ = true;
            }
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Set Model File", GetInstanceName()).c_str())) {
            show_model_dialog_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(CreateControlString("Clear Model File", GetInstanceName()).c_str())) {
            model_path_ = "";
            needs_reinit_ = true;
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
        ImGui::SameLine();
        if (ImGui::Button(CreateControlString("Clear Config File", GetInstanceName()).c_str())) {
            config_path_ = "";
            needs_reinit_ = true;
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
        if (ImGui::Button(CreateControlString("Set Colors File", GetInstanceName()).c_str())) {
            show_colors_dialog_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(CreateControlString("Clear Colors File", GetInstanceName()).c_str())) {
            colors_path_ = "";
            needs_reinit_ = true;
        }
        ImGui::Text("Colors File:");
        if (colors_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", colors_path_.c_str());

        if (show_colors_dialog_)
            ImGui::OpenPopup(CreateControlString("Set Colors", GetInstanceName()).c_str());

        if (colors_dialog_.showFileDialog(CreateControlString("Set Colors", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".txt", &show_colors_dialog_)) {
            colors_path_ = colors_dialog_.selected_path;
            show_colors_dialog_ = false;
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
        ImGui::Checkbox(CreateControlString("RGB", GetInstanceName()).c_str(), &swap_rb_);
        ImGui::Checkbox(CreateControlString("Preprocess Resize", GetInstanceName()).c_str(), &pre_proc_resize_);
    }
}

std::string Segmentation::GetState()
{
    using namespace nlohmann;

    json state;

    state["model_path"] = model_path_;
    state["config_path"] = config_path_;
    state["classes_path"] = classes_path_;
    state["colors_path"] = colors_path_;
    state["mean"] = {mean_[0], mean_[1], mean_[2]};
    state["scale"] = scale_;
    state["preproc"] = pre_proc_resize_;
    json jResize;
    jResize["w"] = model_res_[0];
    jResize["h"] = model_res_[1];
    state["model_res"] = jResize;
    state["swap_rb"] = swap_rb_;
    state["backend"] = (int)current_backend_;
    state["target"] = (int)current_target_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Segmentation::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

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
    if (state.contains("colors_path")) {
        if (!state["colors_path"].empty())
            colors_path_ = state["colors_path"].get<std::string>();
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
    if (!model_path_.empty() && !classes_path_.empty()) {
        std::lock_guard<std::mutex> lk(io_mutex_);
        InitDnn_();
    }
}

}  // End Namespace DSPatch::DSPatchables
