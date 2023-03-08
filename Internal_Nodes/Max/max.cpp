//
// Plugin Max
//

#include "max.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Max::Max() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Max");
    SetComponentCategory_(DSPatch::Category::Category_Merge);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in1", "in2"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    SetEnabled(true);
}

void Max::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<cv::Mat>(1);
    if (!in1 || !in2) {
        return;
    }

    if (!in1->empty() && !in2->empty()) {
        if (IsEnabled()) {
            // Process Image
            if (in1->type() == in2->type() && in1->channels() == in2->channels() && in1->size == in2->size) {
                cv::Mat frame;
                cv::max(*in1, *in2, frame);
                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Max::HasGui(int interface)
{

    return false;
}

void Max::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);
}

std::string Max::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Max::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
}

}  // End Namespace DSPatch::DSPatchables