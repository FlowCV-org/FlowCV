//
// Plugin Normalize
//

#include "normalize.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

static int NormTypeValues[9] = {1,2,4,32};

Normalize::Normalize()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Normalize");
    SetComponentCategory_(DSPatch::Category::Category_Color);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_( 2, {"in", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    norm_type_ = 0;
    alpha_ = 1;
    beta_ = 10;

    SetEnabled(true);

}

void Normalize::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            // Process Image
            if (in2) {
                if (!in2->empty()) {
                    if (NormTypeValues[norm_type_] == cv::NORM_MINMAX)
                        cv::normalize(*in1, frame, alpha_, beta_, NormTypeValues[norm_type_]);
                    else
                        cv::normalize(*in1, frame, alpha_, beta_, NormTypeValues[norm_type_], -1, *in2);
                }
                else
                    cv::normalize(*in1, frame, alpha_, beta_, NormTypeValues[norm_type_]);
            }
            else
                cv::normalize(*in1, frame, alpha_, beta_, NormTypeValues[norm_type_]);

            if (!frame.empty())
                outputs.SetValue(0, frame);
        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Normalize::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Normalize::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Normal Type", GetInstanceName()).c_str(), &norm_type_, "Inf\0L1\0L2\0Min Max\0\0");
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Alpha (Min)", GetInstanceName()).c_str(), &alpha_, 0.01f, 0.0f, 500.0f, "%.2f");
        if (norm_type_ == 3) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Beta (Max)", GetInstanceName()).c_str(), &beta_, 0.01f, 0.0f, 500.0f, "%.2f");
        }

    }

}

std::string Normalize::GetState()
{
    using namespace nlohmann;

    json state;

    state["norm_type"] = norm_type_;
    state["alpha"] = alpha_;
    state["beta"] = beta_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Normalize::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("norm_type"))
        norm_type_ = state["norm_type"].get<int>();
    if (state.contains("alpha"))
        alpha_ = state["alpha"].get<float>();
    if (state.contains("beta"))
        beta_ = state["beta"].get<float>();

}

} // End Namespace DSPatch::DSPatchables