//
// Plugin JsonViewer
//

#include "json_viewer.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

JsonViewer::JsonViewer()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Json_Viewer");
    SetComponentCategory_(DSPatch::Category::Category_Views);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"JSON"}, {DSPatch::IoType::Io_Type_JSON} );

    // 0 outputs
    SetOutputCount_(0);

    show_raw_out_ = false;

    SetEnabled(true);

}

void JsonViewer::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    std::lock_guard<std::mutex> lck (io_mutex_);
    if (!IsEnabled())
        SetEnabled(true);

    // Input 1 Handler
    auto in1 = inputs.GetValue<nlohmann::json>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        // Process JSON
        json_data_ = *in1;
    }

}

bool JsonViewer::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        return true;
    }

    return false;
}

void JsonViewer::UpdateGui(void *context, int interface)
{
    std::lock_guard<std::mutex> lck (io_mutex_);
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        std::string title = "JSON_Viewer_" + std::to_string(GetInstanceCount());
        ImGui::Begin(CreateControlString(title.c_str(), GetInstanceName()).c_str());
        ImGui::Checkbox(CreateControlString("Show Raw Data", GetInstanceName()).c_str(), &show_raw_out_);
        if (show_raw_out_) {
            if (!json_data_.empty()) {
                ImGui::TextUnformatted(json_data_.dump(4).c_str());
                if (ImGui::BeginPopupContextItem("Copy Json")) {
                    if (ImGui::Selectable("Copy JSON to Clipboard")) {
                        ImGui::SetClipboardText(json_data_.dump(4).c_str());
                    }
                    ImGui::EndPopup();
                }
            }
        }
        else {
            if (!json_data_.empty()) {
                if (json_data_.contains("data")) {
                    int colCnt = 0;
                    if (!json_data_["data"].empty()) {
                        for (nlohmann::json::iterator it = json_data_["data"].at(0).begin(); it != json_data_["data"].at(0).end(); ++it)
                            colCnt++;
                        if (colCnt > 0) {
                            ImGui::Columns(colCnt, "json");
                            ImGui::Separator();
                            for (nlohmann::json::iterator it = json_data_["data"].at(0).begin(); it != json_data_["data"].at(0).end(); ++it) {
                                ImGui::Text("%s", it.key().c_str());
                                ImGui::NextColumn();
                            }
                            ImGui::Separator();
                            for (auto &d: json_data_["data"]) {
                                for (nlohmann::json::iterator it = d.begin(); it != d.end(); ++it) {
                                    std::string value;
                                    if (it->is_number_float())
                                        value = std::to_string(it->get<float>());
                                    else if (it->is_number_integer())
                                        value = std::to_string(it->get<int>());
                                    else if (it->is_boolean()) {
                                        if (it->get<bool>())
                                            value = "true";
                                        else
                                            value = "false";
                                    }
                                    else if (it->is_string())
                                        value = it->get<std::string>();
                                    else if (it->is_array())
                                        value = "<Array Data>";
                                    else if (it->is_object())
                                        value = "<JSON Object>";

                                    if (!value.empty()) {
                                        ImGui::Text("%s", value.c_str());
                                    }
                                    ImGui::NextColumn();
                                }
                            }
                            ImGui::Columns(1);
                        }
                    }
                }
            }
        }
        ImGui::End();
    }

}

std::string JsonViewer::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized;

    return stateSerialized;
}

void JsonViewer::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);


}

} // End Namespace DSPatch::DSPatchables