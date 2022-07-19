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

    SetEnabled(true);

}

void RndNoise::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    int depth = CV_8UC3;

    cv::Mat frame;

    std::lock_guard<std::mutex> lck (io_mutex_);
    if (mode_ > 0)
        depth = CV_8U;

    frame = cv::Mat(height_, width_, depth, cv::Scalar::all(0));
    cv::randu(frame, cv::Scalar::all(0), cv::Scalar::all(255));

    if (mode_ == 2)
        cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);

    if (!frame.empty())
        outputs.SetValue(0, frame);
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

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        std::lock_guard<std::mutex> lck (io_mutex_);
        ImGui::Combo(CreateControlString("Mask Mode", GetInstanceName()).c_str(), &mode_, "Color\0BW_8U\0BW_8UC3\0\0");
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &width_, 0.5f, 1, 8000);
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &height_, 0.5f, 1, 8000);
    }

}

std::string RndNoise::GetState()
{
    using namespace nlohmann;

    json state;

    state["mode"] = mode_;
    state["width"] = width_;
    state["height"] = height_;

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

}

} // End Namespace DSPatch::DSPatchables