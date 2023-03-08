//
// Plugin Bitwise
//

#include "bitwise.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Bitwise::Bitwise() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Bitwise");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 3 inputs
    SetInputCount_(3, {"in1", "in2", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    bitwise_mode_ = 0;

    SetEnabled(true);
}

void Bitwise::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    // Get Optional inputs
    auto in2 = inputs.GetValue<cv::Mat>(1);
    auto inMask = inputs.GetValue<cv::Mat>(2);

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame_out, mask;

            // Check Mask
            if (!inMask) {
                mask = cv::Mat(in1->rows, in1->cols, CV_8UC1, 255);
            }
            else {
                inMask->copyTo(mask);
            }

            if (!in2) {
                if (bitwise_mode_ == 0)
                    cv::bitwise_and(*in1, *in1, frame_out, mask);
                else if (bitwise_mode_ == 1)
                    cv::bitwise_not(*in1, frame_out, mask);
                else if (bitwise_mode_ == 2)
                    cv::bitwise_or(*in1, *in1, frame_out, mask);
                else if (bitwise_mode_ == 3)
                    cv::bitwise_xor(*in1, *in1, frame_out, mask);
            }
            else {
                cv::Mat frame2;
                if (in1->size() != in2->size()) {
                    cv::resize(*in2, frame2, in1->size());
                }
                else {
                    in2->copyTo(frame2);
                }

                if (bitwise_mode_ == 0)
                    cv::bitwise_and(*in1, frame2, frame_out, mask);
                else if (bitwise_mode_ == 1)
                    cv::bitwise_not(*in1, frame_out, mask);
                else if (bitwise_mode_ == 2)
                    cv::bitwise_or(*in1, frame2, frame_out, mask);
                else if (bitwise_mode_ == 3)
                    cv::bitwise_xor(*in1, frame2, frame_out, mask);
            }

            outputs.SetValue(0, frame_out);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Bitwise::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Bitwise::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Bitwise Mode", GetInstanceName()).c_str(), &bitwise_mode_, "AND\0NOT\0OR\0XOR\0\0");
    }
}

std::string Bitwise::GetState()
{
    using namespace nlohmann;

    json state;

    state["mode"] = bitwise_mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Bitwise::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("mode"))
        bitwise_mode_ = state["mode"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables