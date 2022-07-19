//
// Plugin Multiply
//

#include "multiply.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Multiply::Multiply()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Multiply");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_( 2, {"in1", "in2"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );\

    scale_ = 1.0f;

    SetEnabled(true);

}

void Multiply::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 || !in2 ) {
        return;
    }

    if (!in1->empty() && !in2->empty()) {
        if (IsEnabled()) {
            // Process Image
            if (in1->type() == in2->type() && in1->channels() == in2->channels() &&
                in1->size == in2->size) {
                cv::Mat frame;
                cv::multiply(*in1, *in2, frame, scale_);
                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }

        } else {
            outputs.SetValue(0, *in1);
        }
    }

}

bool Multiply::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Multiply::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &scale_, 0.001f);
    }
}

std::string Multiply::GetState()
{
    using namespace nlohmann;

    json state;

    state["scale"] = scale_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Multiply::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("scale"))
        scale_ = state["scale"].get<float>();


}

} // End Namespace DSPatch::DSPatchables