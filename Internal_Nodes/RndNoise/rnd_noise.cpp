//
// Plugin RndNoise
//

#include "rnd_noise.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

RndNoise::RndNoise()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Rnd_Noise");
    SetComponentCategory_(DSPatch::Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 0 inputs
    SetInputCount_(0);

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    width_ = 1280;
    height_ = 720;
    mode_ = 0;
    tmp_width_ = width_;
    tmp_height_ = height_;
    tmp_mode_ = mode_;
    fps_ = 30;
    fps_index_ = 7;
    last_time_ = std::chrono::steady_clock::now();
    fps_time_ = (1.0f / (float)fps_) * 1000.0f;
    change_settings_ = false;

    SetEnabled(true);

}

void RndNoise::Process_( SignalBus const& inputs, SignalBus& outputs )
{

    if (io_mutex_.try_lock()) { // Try lock so other threads will skip if locked instead of waiting
        bool should_wait = true;
        while(should_wait) {
            std::chrono::steady_clock::time_point current_time_ = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
            if (delta >= (uint32_t)fps_time_)
                should_wait = false;
        }
        if (change_settings_) {
            width_ = tmp_width_;
            height_ = tmp_height_;
            mode_ = tmp_mode_;
            change_settings_ = false;
        }
        int depth = CV_8UC3;

        cv::Mat frame;
        if (mode_ > 0)
            depth = CV_8U;

        frame = cv::Mat(height_, width_, depth, cv::Scalar::all(0));
        cv::randu(frame, cv::Scalar::all(0), cv::Scalar::all(255));

        if (mode_ == 2)
            cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);

        if (!frame.empty())
            outputs.SetValue(0, frame);
        last_time_ = std::chrono::steady_clock::now();

        io_mutex_.unlock();
    }
}

bool RndNoise::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void RndNoise::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Combo(CreateControlString("Mask Mode", GetInstanceName()).c_str(), &tmp_mode_, "Color\0BW_8U\0BW_8UC3\0\0"))
            change_settings_ = true;
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &tmp_width_, 0.5f, 1, 8000))
            change_settings_ = true;
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &tmp_height_, 0.5f, 1, 8000))
            change_settings_ = true;
        ImGui::SetNextItemWidth(80);
        const int fpsValues[] = {1, 3, 5, 10, 15, 20, 25, 30, 60, 120};
        if (ImGui::Combo(CreateControlString("Output FPS", GetInstanceName()).c_str(), &fps_index_, " 1\0 3\0 5\0 10\0 15\0 20\0 25\0 30\0 60\0 120\0\0")) {
            fps_ = fpsValues[fps_index_];
            fps_time_ = (1.0f / (float)fps_) * 1000.0f;
        }
    }

}

std::string RndNoise::GetState()
{
    using namespace nlohmann;

    json state;

    state["mode"] = mode_;
    state["width"] = width_;
    state["height"] = height_;
    state["fps"] = fps_;
    state["fps_index"] = fps_index_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void RndNoise::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("mode"))
        mode_ = state["mode"].get<int>();
    if (state.contains("width"))
        width_ = state["width"].get<int>();
    if (state.contains("height"))
        height_ = state["height"].get<int>();
    if (state.contains("fps"))
        fps_ = state["fps"].get<int>();
    if (state.contains("fps_index"))
        fps_index_ = state["fps_index"].get<int>();
    change_settings_ = true;

}

} // End Namespace DSPatch::DSPatchables