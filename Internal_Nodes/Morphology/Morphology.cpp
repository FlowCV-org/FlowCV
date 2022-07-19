//
// Plugin Morphology
//

#include "Morphology.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Morphology::Morphology()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Morphology");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    border_color_.x = 0;
    border_color_.y = 0;
    border_color_.z = 0;
    border_type_ = cv::BORDER_DEFAULT;
    iterations_ = 1;
    morph_amt_ = 1;
    morph_shape_ = cv::MORPH_ELLIPSE;
    op_ = 0;

    SetEnabled(true);

}

void Morphology::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            try {
                // Process Image
                cv::Mat element = getStructuringElement( morph_shape_, cv::Size( 2 * morph_amt_ + 1, 2 * morph_amt_ + 1 ), cv::Point( morph_amt_, morph_amt_ ));
                morphologyEx( *in1, frame, op_, element, cv::Point(-1, -1), iterations_, border_type_,
                              cv::Scalar(border_color_.z * 255, border_color_.y * 255, border_color_.x * 255));
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }

            if (!frame.empty())
                outputs.SetValue(0, frame);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Morphology::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Morphology::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Operation", GetInstanceName()).c_str(), &op_, "Erode\0Dilate\0Open\0Close\0Gradient\0Tophat\0Blackhat\0\0");
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Morph Shape", GetInstanceName()).c_str(), &morph_shape_, "Rect\0Cross\0Ellipse\0\0");
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Morph Amt", GetInstanceName()).c_str(), &morph_amt_, 0.25f, 1, 200);
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Iter", GetInstanceName()).c_str(), &iterations_, 0.25f, 1, 50);
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Border Type", GetInstanceName()).c_str(), &border_type_, "Constant\0Replicate\0Reflect\0Wrap\0Default\0Transparent\0\0");
        ImGui::ColorEdit3(CreateControlString("Border Color", GetInstanceName()).c_str(), (float*)&border_color_);
    }

}

std::string Morphology::GetState()
{
    using namespace nlohmann;

    json state;

    state["operation"] = op_;
    state["shape"] = morph_shape_;
    state["amt"] = morph_amt_;
    state["iter"] = iterations_;
    state["border_type"] = border_type_;
    json borderColor;
    borderColor["R"] = border_color_.x;
    borderColor["G"] = border_color_.y;
    borderColor["B"] = border_color_.z;
    state["border_color"] = borderColor;
    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Morphology::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("operation"))
        op_ = state["operation"].get<int>();
    if (state.contains("shape"))
        morph_shape_ = state["shape"].get<int>();
    if (state.contains("amt"))
        morph_amt_ = state["amt"].get<int>();
    if (state.contains("iter"))
        iterations_ = state["iter"].get<int>();
    if (state.contains("border_type"))
        border_type_ = state["border_type"].get<int>();
    if (state.contains("border_color")) {
        border_color_.x = state["border_color"]["R"].get<float>();
        border_color_.y = state["border_color"]["G"].get<float>();
        border_color_.z = state["border_color"]["B"].get<float>();
    }

}

} // End Namespace DSPatch::DSPatchables