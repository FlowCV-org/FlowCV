//
// Video Writer Plugin
//

#include "video_writer.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class VideoWriter
{
};
}  // namespace DSPatch::DSPatchables::internal

const std::vector<std::pair<const std::string, const std::string>> &GetCodecList()
{
    static const auto *codec_list = new std::vector<std::pair<const std::string, const std::string>>{{"H264", ".mp4"}, {"I420", ".mp4"}, {"IYUV", ".mp4"},
        {"M4S2", ".mp4"}, {"MP4S", ".mp4"}, {"MP4V", ".mp4"}, {"MJPG", ".avi"}, {"UYVY", ".avi"}, {"WMV1", ".wmv"}, {"WMV2", ".wmv"}, {"WMV3", ".wmv"},
        {"WMVA", ".wmv"}, {"WMVP", ".wmv"}, {"WVC1", ".wmv"}, {"WVP2", ".wmv"}, {"YUY2", ".mpg"}, {"YV12", ".mpg"}, {"YVU9", ".mpg"}, {"YVYU", ".mpg"}};
    return *codec_list;
}

VideoWriter::VideoWriter() : Component(ProcessOrder::OutOfOrder), p(new internal::VideoWriter())
{

    // Name and Category
    SetComponentName_("Video_Writer");
    SetComponentCategory_(Category::Category_Output);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // add 2 inputs
    SetInputCount_(3, {"in", "fps", "start"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Int, IoType::Io_Type_Bool});

    // add 1 output
    SetOutputCount_(0);

    save_new_file_ = false;
    show_file_dialog_ = false;
    has_fps_in_ = false;
    auto_ext_ = true;
    allow_write_ = false;
    codec_ = 0;
    codec_str_ = "H264";
    last_codec_ = 0;
    fps_ = 30;
    fps_index_ = 7;
    last_fps_ = 30;
    current_time_ = std::chrono::steady_clock::now();
    last_time_ = current_time_;
    fps_time_ = (1.0f / (float)fps_) * 1000.0f;

    SetEnabled(true);
}

VideoWriter::~VideoWriter()
{
    if (video_writer_.isOpened()) {
        video_writer_.release();
    }
}

void VideoWriter::SaveSource()
{
    if (!out_filename_.empty()) {
        if (video_writer_.isOpened())
            video_writer_.release();

        int fourcc = cv::VideoWriter::fourcc(codec_str_[0], codec_str_[1], codec_str_[2], codec_str_[3]);
        if (auto_ext_) {
            auto FourCCList = GetCodecList();
            std::string outExt = FourCCList[codec_].second;
            if (out_filename_.find(outExt) == std::string::npos) {
                auto loc = out_filename_.find_last_of('.');
                if (loc != std::string::npos)
                    out_filename_ = out_filename_.substr(0, loc);
                out_filename_ += outExt;
            }
        }
        // using namespace std::literals;
        fps_time_ = (1.0f / (float)fps_) * 1000.0f;
        video_writer_.open(out_filename_, fourcc, (float)fps_, cv::Size(frame_.cols, frame_.rows), true);
        save_new_file_ = false;
        last_fps_ = fps_;
        last_codec_ = codec_;
    }
}

void VideoWriter::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    static int cnt = 0;

    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<int>(1);
    auto inStart = inputs.GetValue<bool>(2);
    if (!in1)
        return;

    if (in2) {
        has_fps_in_ = true;
        fps_ = *in2;
    }
    else
        has_fps_in_ = false;

    if (inStart) {
        if (*inStart) {
            if (!allow_write_) {
                save_new_file_ = true;
                allow_write_ = true;
                std::cout << "Write Start Auto" << std::endl;
            }
            else {
                allow_write_ = false;
                if (video_writer_.isOpened())
                    video_writer_.release();
                std::cout << "Write Stop Auto" << std::endl;
            }
        }
    }

    // Do something with Input
    if (!in1->empty()) {
        in1->copyTo(frame_);

        if (allow_write_) {
            if (fps_ != last_fps_ || codec_ != last_codec_ || save_new_file_)
                SaveSource();

            if (!video_writer_.isOpened()) {
                SaveSource();
            }
            else {
                current_time_ = std::chrono::steady_clock::now();
                // auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
                // if ((float) delta >= fps_time_) {
                video_writer_.write(frame_);
                last_time_ = current_time_;
                //}
            }
        }
    }
}

bool VideoWriter::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void VideoWriter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(120);
        if (ImGui::Button("Start Write")) {
            save_new_file_ = true;
            allow_write_ = true;
        }
        ImGui::SetNextItemWidth(120);
        ImGui::SameLine();
        if (ImGui::Button("Stop Write")) {
            allow_write_ = false;
            if (video_writer_.isOpened())
                video_writer_.release();
        }
        ImGui::Separator();
        if (allow_write_) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Writing");
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Writing");
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        auto FourCCList = GetCodecList();
        if (ImGui::Combo(
                CreateControlString("Video File Codec", GetInstanceName()).c_str(), &codec_,
                [](void *data, int idx, const char **out_text) {
                    *out_text = ((const std::vector<std::pair<std::string, std::string>> *)data)->at(idx).first.c_str();
                    return true;
                },
                (void *)&FourCCList, (int)FourCCList.size())) {
            codec_str_ = FourCCList[codec_].first;
        }
        if (!has_fps_in_) {
            ImGui::SetNextItemWidth(80);
            const int fpsValues[] = {1, 3, 5, 10, 15, 20, 25, 30, 60, 120};
            if (ImGui::Combo(
                    CreateControlString("Video File FPS", GetInstanceName()).c_str(), &fps_index_, " 1\0 3\0 5\0 10\0 15\0 20\0 25\0 30\0 60\0 120\0\0")) {
                fps_ = fpsValues[fps_index_];
            }
        }
        ImGui::Checkbox(CreateControlString("Auto File Extension", GetInstanceName()).c_str(), &auto_ext_);
        if (ImGui::Button(CreateControlString("Save Movie File", GetInstanceName()).c_str())) {
            show_file_dialog_ = true;
        }
        ImGui::Text("Movie File:");
        if (out_filename_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", out_filename_.c_str());

        if (show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Save Movie", GetInstanceName()).c_str());

        if (file_dialog_.showFileDialog(CreateControlString("Save Movie", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::SAVE,
                ImVec2(700, 310), ".avi,.mpg,.mp4,.mjpg,.mkv,.wmv", &show_file_dialog_)) {
            std::string outExt = FourCCList[codec_].second;
            auto loc = file_dialog_.selected_path.find_last_of('.');
            if (loc == std::string::npos) {
                out_filename_ = file_dialog_.selected_path;
                out_filename_ += outExt;
            }
            else {
                out_filename_ = file_dialog_.selected_path;
            }
            show_file_dialog_ = false;
            save_new_file_ = true;
        }
    }
}

std::string VideoWriter::GetState()
{
    using namespace nlohmann;

    json state;

    state["filename"] = out_filename_;
    state["codec"] = codec_;
    state["codec_str"] = codec_str_;
    state["fps"] = fps_;
    state["fps_index"] = fps_index_;
    state["auto_ext"] = auto_ext_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void VideoWriter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("filename"))
        out_filename_ = state["filename"].get<std::string>();
    if (state.contains("codec"))
        codec_ = state["codec"].get<int>();
    if (state.contains("codec_str"))
        codec_str_ = state["codec_str"].get<std::string>();
    if (state.contains("fps"))
        fps_ = state["fps"].get<int>();
    if (state.contains("fps_index"))
        fps_index_ = state["fps_index"].get<int>();
    if (state.contains("auto_ext"))
        auto_ext_ = state["auto_ext"].get<bool>();
}
