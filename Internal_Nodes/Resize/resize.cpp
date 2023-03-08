//
// Plugin Resize
//

#include "resize.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Resize::Resize() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Resize");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(3, {"in", "ref", "size"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_Int_Array});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    interp_mode_ = 2;
    has_size_input_ = false;
    has_ref_frame_ = false;
    width_ = 512;
    height_ = 512;

    SetEnabled(true);
}

void Resize::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<cv::Mat>(1);
    auto in3 = inputs.GetValue<std::vector<int>>(2);
    if (!in1) {
        return;
    }
    if (!in2) {
        has_ref_frame_ = false;
    }
    else {
        if (!in2->empty())
            has_ref_frame_ = true;
        else
            has_ref_frame_ = false;
    }
    if (!in3) {
        has_size_input_ = false;
    }
    else {
        if (in3->size() == 2)
            has_size_input_ = true;
        else
            has_size_input_ = false;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            if (has_ref_frame_) {
                width_ = in2->cols;
                height_ = in2->rows;
            }
            else if (has_size_input_) {
                width_ = in3->at(0);
                height_ = in3->at(1);
            }

            // Process Image
            if (interp_mode_ == 0)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_NEAREST);
            else if (interp_mode_ == 1)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_LINEAR);
            else if (interp_mode_ == 2)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_CUBIC);
            else if (interp_mode_ == 3)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_AREA);
            else if (interp_mode_ == 4)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_LANCZOS4);
            else if (interp_mode_ == 5)
                cv::resize(*in1, frame, cv::Size(width_, height_), 0, 0, cv::INTER_LINEAR_EXACT);

            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Resize::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Resize::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!has_size_input_ && !has_ref_frame_) {
            ImGui::SetNextItemWidth(100);
            if (ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &width_, 0.5, 1, 10000)) {
                if (width_ < 1)
                    width_ = 1;
            }
            ImGui::SetNextItemWidth(100);
            if (ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &height_, 0.5, 1, 10000)) {
                if (height_ < 1)
                    height_ = 1;
            }
        }
        ImGui::SetNextItemWidth(140);
        ImGui::Combo(
            CreateControlString("Interpolation", GetInstanceName()).c_str(), &interp_mode_, "Nearest\0Bilinear\0BiCubic\0Area\0Lanczos\0Bilinear Exact\0\0");
    }
}

std::string Resize::GetState()
{
    using namespace nlohmann;

    json state;

    state["width"] = width_;
    state["height"] = height_;
    state["mode"] = interp_mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Resize::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("width"))
        width_ = state["width"].get<int>();
    if (state.contains("height"))
        height_ = state["height"].get<int>();
    if (state.contains("mode"))
        interp_mode_ = state["mode"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables