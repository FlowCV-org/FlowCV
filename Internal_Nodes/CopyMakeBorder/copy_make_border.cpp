//
// Plugin CopyMakeBorder
//

#include "copy_make_border.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

CopyMakeBorder::CopyMakeBorder() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Copy_Make_Border");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "size"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Int_Array});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    add_width_ = 0;
    add_height_ = 0;
    border_type_ = 0;
    border_color_ = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    has_size_input_ = false;
    center_ = true;

    SetEnabled(true);
}

void CopyMakeBorder::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<std::vector<int>>(1);
    if (!in1) {
        return;
    }
    if (!in2)
        has_size_input_ = false;
    else {
        if (in2->size() == 2)
            has_size_input_ = true;
        else
            has_size_input_ = false;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            int top = 0;
            int left = 0;
            int borderAmtX = 0;
            int borderAmtY = 0;

            int newWidth = in1->cols + add_width_;
            int newHeight = in1->rows + add_height_;

            if (has_size_input_) {
                newWidth = in2->at(0) + add_width_;
                newHeight = in2->at(1) + add_height_;
            }
            if (center_) {
                borderAmtX = abs(newWidth - in1->cols) * 0.5f;
                borderAmtY = abs(newHeight - in1->rows) * 0.5f;
                top = borderAmtY;
                left = borderAmtX;
            }
            else {
                borderAmtX = abs(newWidth - in1->cols);
                borderAmtY = abs(newHeight - in1->rows);
            }

            if (borderAmtX >= 0 && borderAmtY >= 0) {
                cv::Mat dst;
                copyMakeBorder(*in1, dst, top, borderAmtY, left, borderAmtX, border_type_,
                    cv::Scalar(border_color_.z * 255, border_color_.y * 255, border_color_.x * 255));
                if (!dst.empty())
                    outputs.SetValue(0, dst);
            }
            else {
                outputs.SetValue(0, *in1);
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool CopyMakeBorder::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void CopyMakeBorder::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Checkbox(CreateControlString("Center", GetInstanceName()).c_str(), &center_);
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Border Width", GetInstanceName()).c_str(), &add_width_, 0.5, 0, 500);
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Border Height", GetInstanceName()).c_str(), &add_height_, 0.5, 0, 500);
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Border Type", GetInstanceName()).c_str(), &border_type_, "Constant\0Replicate\0Reflect\0Wrap\0Default\0\0");
        if (border_type_ == 0) {
            ImGui::ColorEdit3(CreateControlString("Border Color", GetInstanceName()).c_str(), (float *)&border_color_);
        }
    }
}

std::string CopyMakeBorder::GetState()
{
    using namespace nlohmann;

    json state;

    state["add_width"] = add_width_;
    state["add_height"] = add_height_;
    state["border_type"] = border_type_;
    json borderColor;
    borderColor["R"] = border_color_.x;
    borderColor["G"] = border_color_.y;
    borderColor["B"] = border_color_.z;
    state["border_color"] = borderColor;
    state["center"] = center_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void CopyMakeBorder::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("add_width"))
        add_width_ = state["add_width"].get<int>();
    if (state.contains("center"))
        center_ = state["center"].get<bool>();
    if (state.contains("add_height"))
        add_height_ = state["add_height"].get<int>();
    if (state.contains("border_type"))
        border_type_ = state["border_type"].get<int>();
    if (state.contains("border_color")) {
        border_color_.x = state["border_color"]["R"].get<float>();
        border_color_.y = state["border_color"]["G"].get<float>();
        border_color_.z = state["border_color"]["B"].get<float>();
    }
}

}  // End Namespace DSPatch::DSPatchables