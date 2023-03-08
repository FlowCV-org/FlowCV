//
// Plugin ScaleAbs
//

#include "scale_abs.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

ScaleAbs::ScaleAbs() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Scale_Abs");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    alpha_ = 1.0;
    beta_ = 0.0;

    SetEnabled(true);
}

void ScaleAbs::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            // Process Image
            cv::convertScaleAbs(*in1, frame, alpha_, beta_);
            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool ScaleAbs::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ScaleAbs::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Alpha", GetInstanceName()).c_str(), &alpha_, 0.01f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Beta", GetInstanceName()).c_str(), &beta_, 0.1f);
    }
}

std::string ScaleAbs::GetState()
{
    using namespace nlohmann;

    json state;

    state["alpha"] = alpha_;
    state["beta"] = beta_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ScaleAbs::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("alpha"))
        alpha_ = state["alpha"].get<float>();
    if (state.contains("beta"))
        beta_ = state["beta"].get<float>();
}

}  // End Namespace DSPatch::DSPatchables