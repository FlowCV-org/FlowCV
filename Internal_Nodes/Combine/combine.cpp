//
// Plugin Combine
//

#include "combine.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Combine::Combine() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Combine");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 4 inputs
    SetInputCount_(4, {"r", "g", "b", "a"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    SetEnabled(true);
}

void Combine::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    // Input 1 Handler
    auto r = inputs.GetValue<cv::Mat>(0);
    auto g = inputs.GetValue<cv::Mat>(1);
    auto b = inputs.GetValue<cv::Mat>(2);
    auto a = inputs.GetValue<cv::Mat>(3);
    if (!r || !g || !b) {
        return;
    }

    // Check Not Empty
    if (!r->empty() || !g->empty() || !b->empty()) {
        // Check Type Match
        if (r->type() != g->type() || r->type() != b->type())
            return;
        // Check Size Match
        if (r->size != g->size || r->size != b->size)
            return;

        std::vector<cv::Mat> channels;

        channels.emplace_back(*b);
        channels.emplace_back(*g);
        channels.emplace_back(*r);

        if (a) {
            if (!a->empty()) {
                if (r->type() == a->type()) {
                    if (r->size == a->size) {
                        channels.emplace_back(*a);
                    }
                }
            }
        }
        cv::Mat out;
        merge(channels, out);

        outputs.SetValue(0, out);
    }
}

bool Combine::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return false;
    }

    return false;
}

void Combine::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);
}

std::string Combine::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Combine::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
}

}  // End Namespace DSPatch::DSPatchables