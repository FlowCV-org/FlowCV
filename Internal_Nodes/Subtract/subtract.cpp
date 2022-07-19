//
// Plugin Subtract
//

#include "subtract.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Subtract::Subtract()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Subtract");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 3 inputs
    SetInputCount_( 3, {"in1", "in2", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    has_mask_ = false;
    mask_mode_ = 0;

    SetEnabled(true);

}

void Subtract::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<cv::Mat>( 1 );
    auto mask = inputs.GetValue<cv::Mat>( 2 );
    if ( !in1 || !in2 ) {
        return;
    }

    has_mask_ = false;
    if (mask)
        if (!mask->empty())
            if (mask->channels() == 1)
                has_mask_ = true;

    if (!in1->empty() && !in2->empty()) {
        if (IsEnabled()) {
            // Process Image
            if (in1->type() == in2->type() && in1->channels() == in2->channels() &&
                in1->size == in2->size) {
                cv::Mat frame;
                if (has_mask_) {
                    if (mask_mode_ == 1)
                        in1->copyTo(frame);
                    else if (mask_mode_ == 2)
                        in2->copyTo(frame);
                }

                if (has_mask_) {
                    cv::subtract(*in1, *in2, frame, *mask);
                }
                else
                    cv::subtract(*in1, *in2, frame);

                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Subtract::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Subtract::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Mask Mode", GetInstanceName()).c_str(), &mask_mode_, "Default\0In1\0In2\0\0");
    }

}

std::string Subtract::GetState()
{
    using namespace nlohmann;

    json state;

    state["mask_mode"] = mask_mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Subtract::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("mask_mode"))
        mask_mode_ = state["mask_mode"].get<int>();


}

} // End Namespace DSPatch::DSPatchables