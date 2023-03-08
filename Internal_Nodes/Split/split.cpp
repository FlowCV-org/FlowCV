//
// Plugin Split
//

#include "split.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Split::Split() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Split");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 4 outputs
    SetOutputCount_(4, {"r", "g", "b", "a"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat});

    is_color_ = false;

    SetEnabled(true);
}

void Split::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        is_color_ = false;
        return;
    }

    if (!in1->empty()) {
        if (in1->channels() < 3) {
            is_color_ = false;
            return;
        }
        is_color_ = true;

        std::vector<cv::Mat> img_planes;
        split(*in1, img_planes);
        outputs.SetValue(0, img_planes[2]);
        outputs.SetValue(1, img_planes[1]);
        outputs.SetValue(2, img_planes[0]);
        if (img_planes.size() == 4)
            outputs.SetValue(3, img_planes[3]);
    }
}

bool Split::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Split::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!is_color_)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Color Image Input Detected");
    }
}

std::string Split::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Split::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
}

}  // End Namespace DSPatch::DSPatchables