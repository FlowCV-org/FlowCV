//
// Plugin DrawText
//

#include "draw_text.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

DrawText::DrawText()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Draw_Text");
    SetComponentCategory_(DSPatch::Category::Category_Draw);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"in", "str"}, {IoType::Io_Type_CvMat, IoType::Io_Type_String} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    memset(text_, '\0', 128);
    text_scale_ = 1.0f;
    text_pos_ = {10, 30};
    text_color_ = {1.0f, 1.0f, 1.0f, 1.0f};
    text_thickness_ = 2;
    has_str_input_ = false;
    text_font_ = 0;

    SetEnabled(true);

}

void DrawText::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<std::string>( 1 );
    if ( !in1 ) {
        return;
    }

    if (!in2)
        has_str_input_ = false;
    else
        has_str_input_ = true;

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            in1->copyTo(frame);

            if (has_str_input_) {
                cv::putText(frame, *in2, text_pos_, text_font_, text_scale_,
                            cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
            }
            else {
                cv::putText(frame, text_, text_pos_, text_font_, text_scale_,
                            cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
            }

            if (!frame.empty())
                outputs.SetValue(0, frame);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool DrawText::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void DrawText::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceName()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!has_str_input_) {
            ImGui::Text("Input:");
            ImGui::InputText(CreateControlString("String", GetInstanceName()).c_str(), text_, 128, ImGuiInputTextFlags_EnterReturnsTrue);
        }
        ImGui::Separator();
        ImGui::Combo(CreateControlString("Font", GetInstanceName()).c_str(), &text_font_,
                     "Simplex\0Plain\0Duplex\0Complex\0Triplex\0Complex Small\0Script Simplex\0Script Complex\0\0");
        ImGui::Separator();
        ImGui::Text("Position:");
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("X", GetInstanceName()).c_str(), &text_pos_.x, 0.5f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Y", GetInstanceName()).c_str(), &text_pos_.y, 0.5f);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &text_scale_, 0.1f);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.1f);
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Color", GetInstanceName()).c_str(), (float*)&text_color_);
    }

}

std::string DrawText::GetState()
{
    using namespace nlohmann;

    json state;

    if (!has_str_input_) {
        state["text"] = text_;
    }
    json pos;
    pos["x"] = text_pos_.x;
    pos["y"] = text_pos_.y;
    state["position"] = pos;
    state["scale"] = text_scale_;
    state["font"] = text_font_;
    state["thickness"] = text_thickness_;
    json tColor;
    tColor["R"] = text_color_.x;
    tColor["G"] = text_color_.y;
    tColor["B"] = text_color_.z;
    state["color"] = tColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DrawText::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    memset(text_, '\0', 128);

    if (state.contains("text")) {
        sprintf(text_, "%s", state["text"].get<std::string>().c_str());
    }
    if (state.contains("color")) {
        text_color_.x = state["color"]["R"].get<float>();
        text_color_.y = state["color"]["G"].get<float>();
        text_color_.z = state["color"]["B"].get<float>();
    }
    if (state.contains("position")) {
        text_pos_.x = state["position"]["x"].get<int>();
        text_pos_.y = state["position"]["y"].get<int>();
    }
    if (state.contains("scale"))
        text_scale_ = state["scale"].get<float>();
    if (state.contains("thickness"))
        text_thickness_ = state["thickness"].get<int>();
    if (state.contains("font"))
        text_font_ = state["font"].get<int>();
}

} // End Namespace DSPatch::DSPatchables