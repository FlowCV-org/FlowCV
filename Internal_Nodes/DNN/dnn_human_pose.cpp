//
// Plugin DNN Human Pose
//

#include "dnn_human_pose.hpp"
#include "dnn_human_pose_helper.hpp"
#include <fstream>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

HumanPose::HumanPose() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Human_Pose");
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
    conf_thresh_ = 0.5f;
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

void HumanPose::InitDnn_()
{
    try {
        net_ = std::make_unique<cv::dnn::Net>(cv::dnn::readNet(model_path_, config_path_, ""));
        net_->setPreferableBackend(current_backend_);
        net_->setPreferableTarget(current_target_);
        out_names_ = net_->getLayerNames();
        std::vector<std::string> layer_types;
        net_->getLayerTypes(layer_types);
        has_concat_output_ = false;
        has_paf_output_ = true;
        for (const auto &lt : layer_types) {
            if (lt.find("Concat") != std::string::npos)
                has_concat_output_ = true;
        }
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

void HumanPose::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    using namespace dnn_human_pose_helper;

    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    std::lock_guard<std::mutex> lk(io_mutex_);
    if (!in1->empty()) {
        if (IsEnabled() && !net_load_error && is_initialized_) {
            // Process Image
            cv::Mat orig, frame, blob, result;
            std::vector<cv::Mat> netOutputParts;
            in1->copyTo(orig);
            cv::Size modelSize = cv::Size(model_res_[0], model_res_[1]);
            std::vector<cv::Mat> outs;
            PoseSizeInfo poseInfo = getPoseInfo(dataset_mode_);
            if (poseInfo.midx == -1) {
                std::cerr << "Can't interpret dataset parameter: " << dataset_mode_ << std::endl;
                return;
            }

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
                blob = cv::dnn::blobFromImage(frame, scale_, modelSize, cv::Scalar(mean_[0], mean_[1], mean_[2]), swap_rb_, false);
                net_->setInput(blob);
                if (has_concat_output_) {
                    result = net_->forward();
                    splitNetOutputBlobToParts(result, cv::Size(frame.cols, frame.rows), netOutputParts);
                }
                else {
                    if (out_names_.size() == 2) {
                        std::vector<cv::Mat> netOutTmp;
                        std::vector<std::vector<cv::Mat>> outBlobArray;
                        net_->forward(outBlobArray, out_names_);
                        result = outBlobArray.at(1).at(0);
                        splitNetOutputBlobToParts(outBlobArray.at(1).at(0), cv::Size(frame.cols, frame.rows), netOutputParts);
                        splitNetOutputBlobToParts(outBlobArray.at(0).at(0), cv::Size(frame.cols, frame.rows), netOutTmp);
                        for (auto &img : netOutTmp) {
                            netOutputParts.emplace_back(std::move(img));
                        }
                    }
                    else {
                        std::cerr << "Wrong number of layers or no Concat Layer" << std::endl;
                        return;
                    }
                }
                if (netOutputParts.size() < 57) {
                    has_paf_output_ = false;
                    force_single_mode_ = true;
                }
            }
            catch (std::exception &e) {
                std::cerr << GetInstanceName() << ", Error Computing DNN" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }
            nlohmann::json json_out;
            nlohmann::json jPoses;
            json_out["data_type"] = "poses";
            nlohmann::json ref;
            ref["w"] = orig.cols;
            ref["h"] = orig.rows;
            json_out["ref_frame"] = ref;

            if (has_paf_output_ && !force_single_mode_) {
                int keyPointId = 0;
                std::vector<std::vector<KeyJoint>> detectedKeypoints;
                std::vector<KeyJoint> keyPointsList;

                for (int i = 0; i < poseInfo.nparts; ++i) {
                    std::vector<KeyJoint> keyPoints;
                    getKeyPoints(netOutputParts[i], 0.1, keyPoints);
                    for (int i = 0; i < keyPoints.size(); ++i, ++keyPointId) {
                        keyPoints.at(i).id = keyPointId;
                    }
                    detectedKeypoints.emplace_back(keyPoints);
                    keyPointsList.insert(keyPointsList.end(), keyPoints.begin(), keyPoints.end());
                }

                if (draw_joints_) {
                    for (int i = 0; i < poseInfo.nparts; ++i) {
                        for (int j = 0; j < detectedKeypoints.at(i).size(); ++j) {
                            cv::circle(frame, detectedKeypoints.at(i).at(j).point, 3, JOINT_COLORS.at(i), -1);
                        }
                    }
                }

                std::vector<std::vector<ValidPair>> validPairs;
                std::set<int> invalidPairs;
                getValidPairs(netOutputParts, detectedKeypoints, validPairs, invalidPairs, conf_thresh_, poseInfo.midx);

                std::vector<std::vector<int>> personwiseKeypoints;
                getPersonwiseKeypoints(validPairs, invalidPairs, personwiseKeypoints, poseInfo.midx);

                for (int i = 0; i < personwiseKeypoints.size(); i++) {
                    nlohmann::json jPose;
                    jPose["key_count"] = personwiseKeypoints.at(i).size();
                    nlohmann::json jKeyPoints;
                    for (int j = 0; j < poseInfo.nparts; j++) {
                        if (j >= POSE_PAIRS.at(poseInfo.midx).size())
                            continue;
                        const std::pair<int, int> &posePair = POSE_PAIRS.at(poseInfo.midx).at(j);
                        if (posePair.first >= personwiseKeypoints.at(i).size() || posePair.second >= personwiseKeypoints.at(i).size())
                            continue;
                        int indexA = personwiseKeypoints.at(i).at(posePair.first);
                        int indexB = personwiseKeypoints.at(i).at(posePair.second);
                        int indexC = personwiseKeypoints.at(i).at(j);
                        nlohmann::json kpPos;
                        nlohmann::json kp;
                        if (indexA == -1 || indexB == -1 || indexC > keyPointsList.size() || indexA >= keyPointsList.size() || indexB >= keyPointsList.size()) {
                            kpPos["x"] = -1;
                            kpPos["y"] = -1;
                            kp[KEYPOINT_NAMES.at(poseInfo.midx).at(j)] = kpPos;
                            jKeyPoints.emplace_back(kp);
                            continue;
                        }
                        const KeyJoint &kpA = keyPointsList.at(indexA);
                        const KeyJoint &kpB = keyPointsList.at(indexB);
                        const KeyJoint &kpI = keyPointsList.at(personwiseKeypoints.at(i).at(j));
                        kpPos["x"] = kpI.point.x;
                        kpPos["y"] = kpI.point.y;
                        kp[KEYPOINT_NAMES.at(poseInfo.midx).at(j)] = kpPos;
                        jKeyPoints.emplace_back(kp);
                        if (draw_joints_)
                            cv::line(frame, kpA.point, kpB.point, JOINT_COLORS.at(j), 2);
                    }
                    jPose["keypoints"] = jKeyPoints;
                    jPoses.emplace_back(jPose);
                }
            }
            else {
                int H = result.size[2];
                int W = result.size[3];
                float SX = float(frame.cols) / (float)W;
                float SY = float(frame.rows) / (float)H;
                // find the position of the body parts
                std::vector<cv::Point> points(poseInfo.nparts);
                nlohmann::json jPose;
                jPose["key_count"] = points.size();
                nlohmann::json jKeyPoints;
                for (int n = 0; n < poseInfo.nparts; n++) {
                    // Slice heatmap of corresponding body's part.
                    cv::Mat heatMap(H, W, CV_32F, result.ptr(0, n));
                    // 1 maximum per heatmap
                    cv::Point p(-1, -1), pm;
                    double conf;
                    minMaxLoc(heatMap, nullptr, &conf, nullptr, &pm);
                    if (conf > conf_thresh_)
                        p = pm;
                    points[n] = p;
                    nlohmann::json kpPos;
                    nlohmann::json kp;
                    if (p.x <= 0 || p.y <= 0) {
                        kpPos["x"] = -1;
                        kpPos["y"] = -1;
                        kp[KEYPOINT_NAMES.at(poseInfo.midx).at(n)] = kpPos;
                        jKeyPoints.emplace_back(kp);
                    }
                    else {
                        kpPos["x"] = (int)(p.x * SX);
                        kpPos["y"] = (int)(p.y * SY);
                        kp[KEYPOINT_NAMES.at(poseInfo.midx).at(n)] = kpPos;
                        jKeyPoints.emplace_back(kp);
                    }
                }
                jPose["keypoints"] = jKeyPoints;
                jPoses.emplace_back(jPose);

                if (draw_joints_) {
                    // connect body parts and draw it !
                    for (int n = 0; n < poseInfo.npairs; n++) {
                        // lookup 2 connected body/hand parts
                        cv::Point2f a = points.at(POSE_PAIRS.at(poseInfo.midx).at(n).first);
                        cv::Point2f b = points.at(POSE_PAIRS.at(poseInfo.midx).at(n).second);

                        // we did not find enough confidence before
                        if (a.x <= 0 || a.y <= 0 || b.x <= 0 || b.y <= 0)
                            continue;

                        // scale to image size
                        a.x *= SX;
                        a.y *= SY;
                        b.x *= SX;
                        b.y *= SY;

                        line(frame, a, b, JOINT_COLORS.at(n), 2);
                        circle(frame, a, 3, cv::Scalar(0, 0, 200), -1);
                        circle(frame, b, 3, cv::Scalar(0, 0, 200), -1);
                    }
                }
            }
            if (pre_proc_resize_) {
                cv::resize(frame, orig, cv::Size(in1->cols, in1->rows));
            }
            if (!jPoses.empty())
                json_out["data"] = jPoses;
            outputs.SetValue(1, json_out);
            outputs.SetValue(0, orig);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool HumanPose::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void HumanPose::UpdateGui(void *context, int interface)
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
        if (!model_path_.empty() || !config_path_.empty()) {
            ImGui::Separator();
            if (ImGui::Button(CreateControlString("Clear Paths", GetInstanceName()).c_str())) {
                model_path_ = "";
                config_path_ = "";
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
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Dataset Mode", GetInstanceName()).c_str(), &dataset_mode_, "COCO\0MPII\0Hand\0\0");
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
        ImGui::Checkbox(CreateControlString("RGB", GetInstanceName()).c_str(), &swap_rb_);
        if (has_paf_output_)
            ImGui::Checkbox(CreateControlString("Force Single Detection Mode", GetInstanceName()).c_str(), &force_single_mode_);
        ImGui::Checkbox(CreateControlString("Preprocess Resize", GetInstanceName()).c_str(), &pre_proc_resize_);
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Joints", GetInstanceName()).c_str(), &draw_joints_);
    }
}

std::string HumanPose::GetState()
{
    using namespace nlohmann;

    json state;

    state["model_path"] = model_path_;
    state["config_path"] = config_path_;
    state["mean"] = {mean_[0], mean_[1], mean_[2]};
    state["scale"] = scale_;
    state["conf_thresh"] = conf_thresh_;
    state["preproc"] = pre_proc_resize_;
    state["force_single"] = force_single_mode_;
    state["dataset_mode"] = dataset_mode_;
    json jResize;
    jResize["w"] = model_res_[0];
    jResize["h"] = model_res_[1];
    state["model_res"] = jResize;
    state["draw_joints"] = draw_joints_;
    state["swap_rb"] = swap_rb_;
    state["backend"] = (int)current_backend_;
    state["target"] = (int)current_target_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void HumanPose::SetState(std::string &&json_serialized)
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

    if (state.contains("mean")) {
        mean_[0] = state["mean"][0].get<float>();
        mean_[1] = state["mean"][1].get<float>();
        mean_[2] = state["mean"][2].get<float>();
    }

    if (state.contains("model_res")) {
        model_res_[0] = state["model_res"]["w"].get<int>();
        model_res_[1] = state["model_res"]["h"].get<int>();
    }

    if (state.contains("force_single"))
        force_single_mode_ = state["force_single"].get<bool>();

    if (state.contains("preproc"))
        pre_proc_resize_ = state["preproc"].get<bool>();

    if (state.contains("scale"))
        scale_ = state["scale"].get<float>();

    if (state.contains("conf_thresh"))
        conf_thresh_ = state["conf_thresh"].get<float>();

    if (state.contains("swap_rb"))
        swap_rb_ = state["swap_rb"].get<bool>();

    if (state.contains("draw_joints"))
        draw_joints_ = state["draw_joints"].get<bool>();

    if (state.contains("backend"))
        current_backend_ = (cv::dnn::Backend)state["backend"].get<int>();

    if (state.contains("dataset_mode"))
        dataset_mode_ = (cv::dnn::Backend)state["dataset_mode"].get<int>();

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
