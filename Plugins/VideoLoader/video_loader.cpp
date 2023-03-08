//
// Plugin MediaReader
//

#include "video_loader.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class VideoLoader
{
};
}  // namespace DSPatch::DSPatchables::internal

enum Play_Mode
{
    Play_Mode_Playing,
    Play_Mode_Stopped
};

VideoLoader::VideoLoader() : Component(ProcessOrder::OutOfOrder), p(new internal::VideoLoader())
{
    // Name and Category
    SetComponentName_("Video_Loader");
    SetComponentCategory_(Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.1");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    play_mode_ = Play_Mode_Playing;
    cur_frame_ = 0;
    loop_ = true;
    show_file_dialog_ = false;
    load_new_file_ = false;
    use_fps_ = true;
    frame_count_ = 0;
    current_time_ = std::chrono::steady_clock::now();
    last_time_ = current_time_;
    start_ = true;

    // 0 inputs
    SetInputCount_(0);

    // 1 outputs
    SetOutputCount_(4, {"video", "start", "frame", "fps"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Bool, IoType::Io_Type_Int, IoType::Io_Type_Int});

    SetEnabled(true);
}

void VideoLoader::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    std::lock_guard<std::mutex> lk(io_mutex_);
    if (load_new_file_) {
        load_new_file_ = false;
        OpenSource();
    }

    if (cap_.isOpened()) {
        cv::Mat frame;
        bool load_next_frame = false;

        if (loop_) {
            if (cur_frame_ >= frame_count_) {
                cap_.set(cv::CAP_PROP_POS_FRAMES, 0);
                cur_frame_ = 0;
                start_ = true;
                last_frame_ = 0;
            }
        }
        else {
            if (cur_frame_ == frame_count_ - 1) {
                play_mode_ = Play_Mode_Stopped;
            }
        }
        if (use_fps_ && play_mode_ == Play_Mode_Playing) {
            bool should_wait = true;
            while (should_wait) {
                current_time_ = std::chrono::steady_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
                if (delta >= (uint32_t)fps_time_)
                    should_wait = false;
            }
            load_next_frame = true;
            last_time_ = current_time_;
        }
        else {

            bool should_wait = true;
            while (should_wait) {
                current_time_ = std::chrono::steady_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
                if (delta >= 16)  // limit to 60 FPS when stopped and not using video FPS
                    should_wait = false;
            }
            load_next_frame = true;
            last_time_ = current_time_;
        }
        if (load_next_frame) {
            if (cur_frame_ == 0 || cur_frame_ == 1)
                start_ = true;
            else
                start_ = false;
            switch (play_mode_) {
                case (int)Play_Mode_Playing:
                    cap_.read(frame);
                    cur_frame_ = (int)cap_.get(cv::CAP_PROP_POS_FRAMES);
                    last_frame_ = cur_frame_;
                    break;
                case (int)Play_Mode_Stopped:
                    cap_.set(cv::CAP_PROP_POS_FRAMES, last_frame_ - 1);
                    cap_.read(frame);
                    cur_frame_ = (int)cap_.get(cv::CAP_PROP_POS_FRAMES);
                    break;
            }
            if (!frame.empty()) {
                outputs.SetValue(0, frame);
                outputs.SetValue(1, start_);
                outputs.SetValue(2, cur_frame_);
                outputs.SetValue(3, fps_);
            }
        }
    }
}

bool VideoLoader::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void VideoLoader::OpenSource()
{
    if (cap_.isOpened())
        cap_.release();

    if (cap_.open(video_file_, cv::CAP_ANY)) {
        cur_frame_ = (int)cap_.get(cv::CAP_PROP_POS_FRAMES);
        last_frame_ = cur_frame_;
        frame_count_ = (int)cap_.get(cv::CAP_PROP_FRAME_COUNT);
        fps_ = (int)cap_.get(cv::CAP_PROP_FPS);
        fps_time_ = (1.0f / (float)fps_) * 1000.0f;
    }
}

void VideoLoader::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Load Movie File", GetInstanceName()).c_str())) {
            show_file_dialog_ = true;
        }
        ImGui::Text("Movie File:");
        if (video_file_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", video_file_.c_str());

        if (show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Load Movie", GetInstanceName()).c_str());

        if (file_dialog_.showFileDialog(CreateControlString("Load Movie", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".avi,.mpg,.mp4,.mkv,.webm", &show_file_dialog_)) {
            video_file_ = file_dialog_.selected_path;
            show_file_dialog_ = false;
            load_new_file_ = true;
        }
        ImGui::Separator();
        ImGui::Text("Movie Controls");
        ImGui::Checkbox(CreateControlString("Loop Playback", GetInstanceName()).c_str(), &loop_);
        ImGui::Checkbox(CreateControlString("Use Movie FPS", GetInstanceName()).c_str(), &use_fps_);
        ImGui::Separator();
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt(CreateControlString("Frame", GetInstanceName()).c_str(), &last_frame_, 0, frame_count_ - 1);
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        if (ImGui::Button(CreateControlString("|<", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            std::lock_guard<std::mutex> lk(io_mutex_);
            last_frame_ = 1;
            cap_.set(cv::CAP_PROP_POS_FRAMES, last_frame_);
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        if (ImGui::Button(CreateControlString("<<", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            last_frame_--;
            if (last_frame_ < 1)
                last_frame_ = 1;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        if (play_mode_ == Play_Mode_Stopped) {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        }
        if (ImGui::Button(CreateControlString("[]", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            play_mode_ = Play_Mode_Stopped;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        if (play_mode_ == Play_Mode_Playing) {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.285f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.285f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.285f, 0.8f, 0.8f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        }
        if (ImGui::Button(CreateControlString(">", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            play_mode_ = Play_Mode_Playing;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        if (ImGui::Button(CreateControlString(">>", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            last_frame_++;
            if (last_frame_ > frame_count_ - 1)
                last_frame_ = frame_count_ - 1;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        if (ImGui::Button(CreateControlString(">|", GetInstanceName()).c_str(), ImVec2(32, 32))) {
            last_frame_ = frame_count_ - 1;
        }
        ImGui::PopStyleColor(3);
    }
}

std::string VideoLoader::GetState()
{
    using namespace nlohmann;

    json state;

    state["movie_path"] = video_file_;
    state["looping"] = loop_;
    state["use_fps_speed"] = use_fps_;
    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void VideoLoader::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("looping"))
        loop_ = state["looping"].get<bool>();
    if (state.contains("use_fps_speed"))
        use_fps_ = state["use_fps_speed"].get<bool>();
    if (state.contains("movie_path")) {
        if (!state["movie_path"].empty()) {
            video_file_ = state["movie_path"].get<std::string>();
            load_new_file_ = true;
        }
    }
}
