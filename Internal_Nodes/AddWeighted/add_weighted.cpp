//
// Plugin AddWeighted
//

#include "add_weighted.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

AddWeighted::AddWeighted()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Add_Weighted");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"in1", "in2"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    alpha_ = 0.5f;
    beta_ = 0.5f;
    gamma_ = 0.0f;

    SetEnabled(true);

}

void AddWeighted::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 || !in2 ) {
        return;
    }

    if (!in1->empty() && !in2->empty()) {
        if (IsEnabled()) {
            // Process Image
            if (in1->type() == in2->type() &&
            in1->channels() == in2->channels() &&
            in1->size == in2->size) {
                cv::Mat frame;
                cv::addWeighted(*in1, alpha_, *in2, beta_, gamma_, frame);

                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }

        } else {
            outputs.SetValue(0, *in1);
        }
    }

}

bool AddWeighted::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void AddWeighted::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Alpha", GetInstanceName()).c_str(), &alpha_, 0.05f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Beta", GetInstanceName()).c_str(), &beta_, 0.05f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Gamma", GetInstanceName()).c_str(), &gamma_, 0.05f);
    }

}

std::string AddWeighted::GetState()
{
    using namespace nlohmann;

    json state;

    state["alpha"] = alpha_;
    state["beta"] = beta_;
    state["gamma"] = gamma_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void AddWeighted::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("alpha"))
        alpha_ = state["alpha"].get<float>();
    if (state.contains("beta"))
        beta_ = state["beta"].get<float>();
    if (state.contains("gamma"))
        gamma_ = state["gamma"].get<float>();

}

} // End Namespace DSPatch::DSPatchables