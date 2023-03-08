//
// Plugin Add
//

#include "add.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Add::Add() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Add");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 3 inputs
    SetInputCount_(3, {"in1", "in2", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    mask_mode_ = 0;

    SetEnabled(true);
}

void Add::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<cv::Mat>(1);
    auto mask = inputs.GetValue<cv::Mat>(2);
    if (!in1 || !in2) {
        return;
    }

    bool has_mask = false;
    if (mask)
        if (!mask->empty())
            if (mask->channels() == 1)
                has_mask = true;

    if (!in1->empty() && !in2->empty()) {
        if (IsEnabled()) {
            // Process Image
            if (in1->type() == in2->type() && in1->channels() == in2->channels() && in1->size == in2->size) {
                cv::Mat frame;
                if (has_mask) {
                    if (mask_mode_ == 0) {  // Default - Fill with black
                        if (!frame.empty())
                            frame.setTo(cv::Scalar(0, 0, 0));
                    }
                    else if (mask_mode_ == 1)
                        in1->copyTo(frame);
                    else if (mask_mode_ == 2)
                        in2->copyTo(frame);
                }

                if (has_mask) {
                    cv::add(*in1, *in2, frame, *mask);
                }
                else
                    cv::add(*in1, *in2, frame);

                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Add::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Add::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Mask Mode", GetInstanceName()).c_str(), &mask_mode_, "Default\0In1\0In2\0\0");
    }
}

std::string Add::GetState()
{
    using namespace nlohmann;

    json state;

    state["mask_mode"] = mask_mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Add::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("mask_mode"))
        mask_mode_ = state["mask_mode"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables