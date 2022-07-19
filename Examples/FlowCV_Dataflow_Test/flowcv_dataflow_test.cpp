//
// Simple FlowCV Data Flow Example
//

#include <iostream>
#include <filesystem>
#include <DSPatch.h>
#include <FlowCV_Manager.hpp>
#include "FlowCV_Types.hpp"
#include "imgui_internal.h"
#include "imgui_wrapper.hpp"
#include "json.hpp"

using namespace DSPatch;

void PrintNodeConnectionInfo(FlowCV::FlowCV_Manager& fm)
{
        for (int i = 0; i < fm.GetNodeCount(); i++) {
            FlowCV::NodeInfo ni;
            fm.GetNodeInfoByIndex(i, ni);
            std::cout << "--------------------------------------" << std::endl;
            std::cout << "  Name: " << ni.desc.name << std::endl;
            std::cout << "  ID: " << ni.id << std::endl;
            std::vector<FlowCV::Wire> connections = fm.GetNodeConnectionsFromIndex(i);
            std::cout << "  Connections: " << std::endl;
            for (const auto &wire : connections) {
                if (wire.from.id == ni.id)
                    std::cout << "      out: " << wire.from.index << " --> in: " << wire.to.index << " ID: " << wire.to.id << std::endl;
                if (wire.to.id == ni.id)
                    std::cout << "      in: " << wire.to.index << " <-- out: " << wire.from.index << " ID: " << wire.from.id << std::endl;
            }
            std::cout << "--------------------------------------" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    using namespace FlowCV;

    FlowCV_Manager flowMan;

    std::string pluginDir;
    pluginDir = std::filesystem::current_path().string();
    pluginDir += std::filesystem::path::preferred_separator;
    pluginDir += "Plugins";

    if (std::filesystem::exists(pluginDir))
        flowMan.plugin_manager_->LoadPlugins(pluginDir.c_str());

    uint32_t pCount = flowMan.plugin_manager_->PluginCount();
    std::cout << "External Plugin Count: " << pCount << std::endl;
    std::cout << "Plugin List: " << std::endl;
    auto categories = getCategories();
    for (int i = 0; i < pCount; i++) {
        NodeDescription plgNodeDesc;
        if (flowMan.plugin_manager_->GetPluginDescription(i, plgNodeDesc)) {
            std::cout << "--------------------------------" << std::endl;
            std::cout << "    Name: " << plgNodeDesc.name << std::endl;
            std::cout << "    Category: " << categories[plgNodeDesc.category] << std::endl;
            std::cout << "    Author: " << plgNodeDesc.author << std::endl;
            std::cout << "    Version: " << plgNodeDesc.version << std::endl;
            std::cout << "    Inputs: " << plgNodeDesc.input_count << std::endl;
            std::cout << "    Outputs: " << plgNodeDesc.output_count << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
    }
    std::cout << std::endl;

    uint32_t iCount = flowMan.internal_node_manager_->NodeCount();
    std::cout << "Internal Node Count: " << iCount << std::endl;
    std::cout << "Node List: " << std::endl;
    for (int i = 0; i < iCount; i++) {
        NodeDescription intNodeDesc;
        if (flowMan.internal_node_manager_->GetNodeDescription(i, intNodeDesc)) {
            std::cout << "--------------------------------" << std::endl;
            std::cout << "    Name: " << intNodeDesc.name << std::endl;
            std::cout << "    Category: " << categories[intNodeDesc.category] << std::endl;
            std::cout << "    Author: " << intNodeDesc.author << std::endl;
            std::cout << "    Version: " << intNodeDesc.version << std::endl;
            std::cout << "    Inputs: " << intNodeDesc.input_count << std::endl;
            std::cout << "    Outputs: " << intNodeDesc.output_count << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
    }
    std::cout << std::endl;

    // Create and Connect Nodes Manually
    uint64_t capId = flowMan.CreateNewNodeInstance("Video_Capture");
    uint64_t blurId = flowMan.CreateNewNodeInstance("Blur");
    uint64_t viewId = flowMan.CreateNewNodeInstance("Viewer");
    uint64_t viewId2 = flowMan.CreateNewNodeInstance("Viewer");
    uint64_t writeId = flowMan.CreateNewNodeInstance("Video_Writer");

    // Configure initial state
    nlohmann::json blurState;
    blurState["lock_h_v"] = true;
    blurState["blur_mode"] = 1;
    blurState["blur_amt_h"] = 5.0f;
    NodeInfo ni;
    if (flowMan.GetNodeInfoById(blurId, ni))
        ni.node_ptr->SetState(blurState.dump());

    nlohmann::json camState;
    camState["list_index"] = 1;
    camState["src_index"] = 1;
    camState["src_mode"] = 2;
    if (flowMan.GetNodeInfoById(capId, ni)) {
        ni.node_ptr->SetState(camState.dump());
    }

    // Connect inputs and outputs
    flowMan.ConnectNodes(capId, 0, blurId, 0);
    flowMan.ConnectNodes(blurId, 0, viewId, 0);
    flowMan.ConnectNodes(capId, 0, viewId2, 0);
    flowMan.ConnectNodes(blurId, 0, writeId, 0);

    // Save State to file
//    cm.SaveState("./simple_test.flow");

    // Load State from File
//    cm.LoadState(pm, im, "./simple_test.flow");

    std::cout << "Flow Node Count: " << flowMan.GetNodeCount() << std::endl;
    PrintNodeConnectionInfo(flowMan);
//    nlohmann::json state = flowMan.GetState();
//    std::cout << state.dump(4) << std::endl;

    ImGuiWrapper imgui;
    imgui.Init(1280, 720, "Test GUI");
    ImGuiIO &io = ImGui::GetIO();

    bool initialized = false;
    ImGuiID dock_main_id, dock_id_prop;

    // Run Flow Graph in Background Thread
    flowMan.StartAutoTick();

    // Main loop for handling UI
    while(!imgui.ShouldClose()) {
        //cm.Tick();
        ImGuiWrapper::PollEvents();
        ImGuiWrapper::NewFrame();

        ImGuiWrapper::StartDockSpace(true);
        ImGuiID dockspace_id = ImGui::GetID("InvisibleWindowDockSpace");

        // Start UI
        {
            if (!initialized)
            {
                initialized = true;

                dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
                dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);

                ImGui::DockBuilderFinish(dockspace_id);
            }

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Load", "CTRL+L")) {
                        flowMan.StopAutoTick();
                        flowMan.LoadState("./simple_test.flow");
                        flowMan.StartAutoTick();
                    }
                    if (ImGui::MenuItem("Save", "CTRL+S")) {flowMan.SaveState("./simple_test.flow");}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "CTRL+Q")) { break; }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                    if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // Process Nodes Controls UI
            ImGui::SetNextWindowDockID(dock_id_prop, ImGuiCond_FirstUseEver);
            ImGui::Begin("Properties");
            for (int i = 0; i < flowMan.GetNodeCount(); i++) {
                if (flowMan.NodeHasUI(i, GuiInterfaceType_Controls)) {
                    NodeInfo nInfo;
                    if (flowMan.GetNodeInfoByIndex(i, nInfo)) {
                        std::string param_name = nInfo.desc.name;
                        param_name += "_";
                        param_name += std::to_string(nInfo.node_ptr->GetInstanceCount());
                        if (ImGui::CollapsingHeader(param_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                            flowMan.ProcessNodeUI(i, imgui.GetImGuiCurrentContext(), GuiInterfaceType_Controls);
                        }
                        ImGui::Separator();
                    }
                }
            }
            ImGui::End();

            // Process Nodes Main UI
            for (int i = 0; i < flowMan.GetNodeCount(); i++) {
                if (flowMan.NodeHasUI(i, GuiInterfaceType_Main)) {
                    NodeInfo nInfo;
                    if (flowMan.GetNodeInfoByIndex(i, nInfo)) {
                        if (nInfo.desc.name == "Viewer")
                            ImGui::SetNextWindowDockID(dock_main_id, ImGuiCond_FirstUseEver);
                        flowMan.ProcessNodeUI(i, imgui.GetImGuiCurrentContext(), GuiInterfaceType_Main);
                    }
                }
            }

        }
        ImGuiWrapper::FrameEnd();
        imgui.Update();
    }

    flowMan.StopAutoTick();

    return 0;
}