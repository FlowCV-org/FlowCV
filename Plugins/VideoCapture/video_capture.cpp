//
// Simple Video Capture Plugin
//

#include "video_capture.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class VideoCapture
{
};
}  // namespace DSPatch::DSPatchables::internal

const std::map<int, const char *> &GetPropertyList()
{
    static const auto *property_list =
        new std::map<int, const char *>{{10, "Brightness"}, {11, "Contrast"}, {12, "Saturation"}, {13, "Hue"}, {14, "Gain"}, {15, "Exposure"},
            {20, "Sharpness"}, {21, "Auto Exposure"}, {22, "Gamma"}, {27, "Zoom"}, {28, "Focus"}, {30, "ISO Speed"}, {31, "Backlight"}, {35, "Iris"}};

    return *property_list;
}

VideoCapture::VideoCapture() : Component(ProcessOrder::InOrder), p(new internal::VideoCapture())
{
    // Name and Category
    SetComponentName_("Video_Capture");
    SetComponentCategory_(Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // Setup Supported Video Modes
    video_modes_.emplace_back(VideoMode{320, 240});
    video_modes_.emplace_back(VideoMode{640, 480});
    video_modes_.emplace_back(VideoMode{800, 600});
    video_modes_.emplace_back(VideoMode{1024, 768});
    video_modes_.emplace_back(VideoMode{1280, 720});
    video_modes_.emplace_back(VideoMode{1280, 960});
    video_modes_.emplace_back(VideoMode{1600, 1200});
    video_modes_.emplace_back(VideoMode{1920, 1080});
    video_modes_.emplace_back(VideoMode{2048, 1536});
    video_modes_.emplace_back(VideoMode{2592, 1944});
    video_modes_.emplace_back(VideoMode{3840, 2160});

    SetInputCount_(0);

    SetOutputCount_(2, {"src", "fps"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Int});

    cam_enum_.RefreshCameraList();

    fps_ = 30;
    last_fps_ = 30;
    fps_index_ = 7;
    force_index_ = false;
    first_load_set_ = false;
    restore_man_set_ = false;
    src_mode_ = 4;
    last_mode_ = 4;
    src_index_ = 0;
    list_index_ = 0;
    last_index_ = 0;

    SetEnabled(true);
}

VideoCapture::~VideoCapture()
{
    if (!frame_.empty())
        frame_.release();
    if (cap_.isOpened())
        cap_.release();
}

void VideoCapture::OpenSource()
{
    if (src_index_ > 0) {
        std::lock_guard<std::mutex> lck(io_mutex_);
        if (cap_.isOpened())
            cap_.release();

        int back_end = cv::CAP_ANY;

#ifdef _WIN32
        back_end = cv::CAP_DSHOW;
#endif

#ifdef __linux__
        back_end = cv::CAP_V4L;
#endif

#ifdef __APPLE__
        back_end = cv::CAP_ANY;
#endif

        try {
            if (cap_.open(src_index_ - 1, back_end)) {
                cap_.set(cv::CAP_PROP_FRAME_WIDTH, video_modes_.at(src_mode_).width);
                cap_.set(cv::CAP_PROP_FRAME_HEIGHT, video_modes_.at(src_mode_).height);
                cap_.set(cv::CAP_PROP_FPS, fps_);
                fps_ = (int)cap_.get(cv::CAP_PROP_FPS);
                last_index_ = src_index_;
                last_mode_ = src_mode_;
                last_fps_ = fps_;
                GetCameraProperties();
            }
        }
        catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
}

void VideoCapture::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    if (src_index_ != last_index_ || src_mode_ != last_mode_ || fps_ != last_fps_) {
        OpenSource();
    }

    if (!cap_.isOpened())
        OpenSource();

    if (cap_.isOpened()) {
        if (first_load_set_)
            SetCameraProperties();
        if (cap_.read(frame_)) {
            if (!frame_.empty()) {
                outputs.SetValue(0, frame_);
                outputs.SetValue(1, fps_);
            }
        }
    }
}

bool VideoCapture::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void VideoCapture::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Refresh Cap List", GetInstanceName()).c_str())) {
            std::string curName = cam_enum_.GetCameraName(list_index_);
            cam_enum_.RefreshCameraList();
            if (curName != cam_enum_.GetCameraName(list_index_)) {
                bool foundMatch = false;
                for (int i = 0; i < cam_enum_.GetCameraCount(); i++) {
                    if (curName == cam_enum_.GetCameraName(i)) {
                        list_index_ = i;
                        src_index_ = cam_enum_.GetCameraIndex(i);
                        foundMatch = true;
                        break;
                    }
                }
                if (!foundMatch) {
                    list_index_ = 0;
                    src_index_ = 0;
                }
            }
        }
        std::vector<std::string> cams;
        cams.reserve(cam_enum_.GetCameraCount());
        for (int i = 0; i < cam_enum_.GetCameraCount(); i++) {
            cams.emplace_back(cam_enum_.GetCameraName(i));
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Force Use Index", GetInstanceName()).c_str(), &force_index_);
        ImGui::SetNextItemWidth(200);
        if (ImGui::Combo(
                CreateControlString("Camera", GetInstanceName()).c_str(), &list_index_,
                [](void *data, int idx, const char **out_text) {
                    *out_text = ((const std::vector<std::string> *)data)->at(idx).c_str();
                    return true;
                },
                (void *)&cams, (int)cams.size())) {
            src_index_ = cam_enum_.GetCameraIndex(list_index_);
        }

        std::vector<std::string> modes;
        for (auto m : video_modes_) {
            std::string res;
            res += std::to_string(m.width);
            res += " x ";
            res += std::to_string(m.height);
            modes.emplace_back(res);
        }
        ImGui::SetNextItemWidth(100);
        ImGui::Combo(
            CreateControlString("Video Resolution", GetInstanceName()).c_str(), &src_mode_,
            [](void *data, int idx, const char **out_text) {
                *out_text = ((const std::vector<std::string> *)data)->at(idx).c_str();
                return true;
            },
            (void *)&modes, (int)modes.size());
        ImGui::SetNextItemWidth(80);
        const int fpsValues[] = {1, 3, 5, 10, 15, 20, 25, 30, 60, 120};
        if (ImGui::Combo(CreateControlString("Video FPS", GetInstanceName()).c_str(), &fps_index_, " 1\0 3\0 5\0 10\0 15\0 20\0 25\0 30\0 60\0 120\0\0")) {
            fps_ = fpsValues[fps_index_];
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Open External Settings", GetInstanceName()).c_str())) {
            cap_.set(cv::CAP_PROP_SETTINGS, src_index_);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Restore Settings on Load", GetInstanceName()).c_str(), &restore_man_set_);
        if (ImGui::TreeNode("Manual Settings")) {
            for (auto &prop : cam_props_) {
                ImGui::SetNextItemWidth(100);
                if (ImGui::DragFloat(CreateControlString(prop.second.name.c_str(), GetInstanceName()).c_str(), &prop.second.value, 1.0f)) {
                    try {
                        if (cap_.isOpened())
                            cap_.set(prop.first, (double)prop.second.value);
                    }
                    catch (const std::exception &e) {
                        std::cout << "Error: " << e.what() << std::endl;
                    }
                }
            }
            if (ImGui::Button(CreateControlString("Refresh Settings", GetInstanceName()).c_str())) {
                GetCameraProperties();
            }
            ImGui::TreePop();
        }
    }
}

std::string VideoCapture::GetState()
{
    using namespace nlohmann;

    json state;

    state["src_index"] = src_index_;
    state["list_index"] = list_index_;
    state["force_index"] = force_index_;
    state["src_name"] = cam_enum_.GetCameraName(list_index_);
    state["src_mode"] = src_mode_;
    state["fps"] = fps_;
    state["fps_index"] = fps_index_;
    state["set_load_man"] = restore_man_set_;
    json camSettings;
    for (auto &prop : cam_props_) {
        camSettings[prop.second.name] = prop.second.value;
    }
    state["camera_settings"] = camSettings;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void VideoCapture::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("list_index")) {
        if (state["list_index"].is_number()) {
            list_index_ = state["list_index"].get<int>();
        }
    }
    if (state.contains("force_index")) {
        force_index_ = state["force_index"].get<bool>();
    }
    if (state.contains("src_index")) {
        if (state["src_index"].is_number()) {
            src_index_ = state["src_index"].get<int>();
        }
    }
    if (state.contains("src_mode")) {
        if (state["src_mode"].is_number()) {
            src_mode_ = state["src_mode"].get<int>();
        }
    }
    if (state.contains("fps") && state.contains("fps_index")) {
        fps_ = state["fps"].get<int>();
        fps_index_ = state["fps_index"].get<int>();
    }
    if (state.contains("src_name")) {
        if (!force_index_) {
            std::string camName = state["src_name"].get<std::string>();
            cam_enum_.RefreshCameraList();
            if (camName != cam_enum_.GetCameraName(list_index_)) {
                bool foundMatch = false;
                for (int i = 0; i < cam_enum_.GetCameraCount(); i++) {
                    if (camName == cam_enum_.GetCameraName(i)) {
                        list_index_ = i;
                        src_index_ = cam_enum_.GetCameraIndex(i);
                        foundMatch = true;
                        break;
                    }
                }
                if (!foundMatch) {
                    list_index_ = 0;
                    src_index_ = 0;
                }
            }
        }
    }
    if (state.contains("camera_settings")) {
        loaded_settings_ = state["camera_settings"];
    }
    if (state.contains("set_load_man")) {
        restore_man_set_ = state["set_load_man"].get<bool>();
        if (restore_man_set_)
            first_load_set_ = true;
    }
}

void VideoCapture::GetCameraProperties()
{
    cam_props_.clear();

    auto propList = GetPropertyList();

    for (auto &prop : propList) {
        camera_property_info cpi;
        double val = cap_.get((cv::VideoCaptureProperties)prop.first);
        // if ((int)val != 0 && (int)val != -1) {
        cpi.value = (float)val;
        cpi.name = prop.second;
        cpi.supported = true;
        cam_props_[prop.first] = cpi;
        //}
    }
}

void VideoCapture::SetCameraProperties()
{
    if (cap_.isOpened()) {
        first_load_set_ = false;
        if (restore_man_set_) {
            if (!loaded_settings_.empty()) {
                for (auto &el : loaded_settings_.items()) {
                    for (auto &prop : cam_props_) {
                        if (prop.second.name == el.key()) {
                            if ((int)prop.second.value != (int)el.value().get<float>()) {
                                cap_.set(prop.first, el.value().get<float>());
                            }
                        }
                    }
                }
                GetCameraProperties();
            }
        }
    }
}
