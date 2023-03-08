//
// Plugin Template
//

#include "plugin_template.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class PluginName
{
};
}  // namespace DSPatch::DSPatchables::internal

PluginName::PluginName() : Component(ProcessOrder::OutOfOrder), p(new internal::PluginName())
{
    // Name and Category
    SetComponentName_("Plugin_Name");
    SetComponentCategory_(Category::Category_Other);
    SetComponentAuthor_("Author");
    SetComponentVersion_("0.0.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    // Add Props Here, Bool (checkbox), Int, Float, Options (ComboBox)
    // Example:
    // props_.AddInt("key", "description", init_default_value, min_value, max_value, step_value, ui_visible);

    SetEnabled(true);
}

void PluginName::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Handle Input Code Here

    // Thread safe property sync from UI
    props_.Sync();

    // Handle Output Code Here
}

bool PluginName::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void PluginName::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        // Draw Property Controls (will be drawn in order added)
        props_.DrawUi(GetInstanceName());

        // Add additional UI property logic here
    }
}

std::string PluginName::GetState()
{
    using namespace nlohmann;

    json state;

    // Convert properties to JSON
    props_.ToJson(state);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void PluginName::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    // Set Properties from JSON
    props_.FromJson(state);
}
