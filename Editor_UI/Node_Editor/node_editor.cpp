# include <application.h>
#include "widgets.h"
# include <imgui_node_editor.h>
# include <imgui_internal.h>
#include <iostream>
#include <vector>
#include <chrono>

#define IN_OFFSET 99
#define OUT_OFFSET 199

namespace ed = ax::NodeEditor;

struct DerivedPinInfo
{
    uint64_t nodeId{};
    int32_t pinNum{};
    bool isInput{};
};

struct EditorGlobals {
    ed::EditorContext *g_Context;    // Editor context, required to trace a editor state.
    bool g_FirstFrame;    // Flag set for first frame only, some action need to be executed once.
    int g_NewState;
    bool g_SetNewNodePos;
    bool g_ShowTabSearchWin;
    ImVec2 g_NewNodePos;
    nlohmann::json g_currentState;
    nlohmann::json g_lastState;
    bool createNewNode = false;
    ed::NodeId contextNodeId;
    ed::PinId newNodeLinkPin;
};

EditorGlobals* GetEditorGlobals()
{
    static auto editor_globals = new EditorGlobals{
        nullptr,        // g_Context
        true,           // g_FirstFrame
        3,              // g_NewState
        true,           // g_SetNewNodePos
        false,          // g_ShowTabSearchWin
        {0, 0},         // g_NewNodePos
        nullptr,        // g_currentState
        nullptr,        // g_lastState
        false,          // createNewNode
        0,              // contextNodeId
        0               // newNodeLinkPin
    };

    return editor_globals;
}

bool FindStringCaseInsensitive(const std::string & str, const std::string & strSearch)
{
    auto it = std::search(
        str.begin(), str.end(),
        strSearch.begin(),   strSearch.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return (it != str.end() );
}

const char* Application_GetName()
{
    return "FlowCV - Node Editor";
}

void Application_Initialize(std::string& appPath)
{
    ed::Config config;
    static std::string editorConfig = appPath + "/NodeEditorState.json";
    config.SettingsFile = editorConfig.c_str();
    GetEditorGlobals()->g_Context = ed::CreateEditor(&config);
}

void Application_Finalize()
{
    ed::DestroyEditor(GetEditorGlobals()->g_Context);
    delete GetApplicationGlobals();
    delete GetEditorGlobals();
}

void ImGuiEx_BeginColumn()
{
    ImGui::BeginGroup();
}

void ImGuiEx_NextColumn()
{
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
}

void ImGuiEx_EndColumn()
{
    ImGui::EndGroup();
}

void CopySelectedNodes(FlowCV::FlowCV_Manager& flowMan)
{
    std::vector<ed::NodeId> selectedNodes;
    selectedNodes.resize(ed::GetSelectedObjectCount());
    int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
    nlohmann::json jNodes;
    nlohmann::json jConn;
    nlohmann::json jEdit;
    nlohmann::json jClipCopy;
    for (const auto &node: selectedNodes) {
        FlowCV::NodeInfo ni;
        // Get Node Info
        if (flowMan.GetNodeInfoById(node.Get(), ni)) {
            std::string strState = ni.node_ptr->GetState();
            if (!strState.empty()) {
                nlohmann::json jNode;
                jNode["id"] = ni.id;
                jNode["name"] = ni.desc.name;
                nlohmann::json jParams = nlohmann::json::parse(strState);
                jNode["params"] = jParams;
                jNodes.emplace_back(jNode);
            }
            // Get Wiring Info
            std::vector<FlowCV::Wire> wireConn = flowMan.GetNodeConnectionsFromId(ni.id);
            for (const auto &wire: wireConn) {
                nlohmann::json w;
                if (wire.from.id == ni.id) {
                    // Make sure to node is in selected
                    bool hasToNode = false;
                    for (auto &nodeCh : selectedNodes) {
                        if (nodeCh.Get() == wire.to.id)
                            hasToNode = true;
                    }
                    if (hasToNode) {
                        w["from_id"] = wire.from.id;
                        w["from_idx"] = wire.from.index;
                        w["to_id"] = wire.to.id;
                        w["to_idx"] = wire.to.index;
                        jConn.push_back(w);
                    }
                }
            }
            // Get Editor Info For new Relative Paste Location
            nlohmann::json jNode;
            auto nodePos = ed::GetNodePosition((ed::NodeId)ni.id);
            nlohmann::json loc;
            jNode["id"] = ni.id;
            loc["x"] = nodePos.x;
            loc["y"] = nodePos.y;
            jNode["location"] = loc;
            jEdit.emplace_back(jNode);
        }
    }
    jClipCopy["nodes"] = jNodes;
    if (!jConn.empty())
        jClipCopy["connections"] = jConn;
    if (jEdit.size() > 1)
        jClipCopy["editor"] = jEdit;

    ImGui::SetClipboardText(jClipCopy.dump(4).c_str());
}

void CutSelectedNodes(FlowCV::FlowCV_Manager& flowMan)
{
    CopySelectedNodes(flowMan);
    std::vector<ed::NodeId> selectedNodes;
    selectedNodes.resize(ed::GetSelectedObjectCount());
    int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
    ed::Suspend();
    flowMan.StopAutoTick();
    for (const auto &node: selectedNodes) {
        ed::NodeId id = node.Get();
        flowMan.RemoveNodeInstance((uint64_t)id.Get());
    }
    ed::Resume();
    flowMan.StartAutoTick();
}

void PasteNodes(FlowCV::FlowCV_Manager& flowMan)
{
    try {
        nlohmann::json state = nlohmann::json::parse(ImGui::GetClipboardText());

        ImVec2 curMousePos = ImGui::GetMousePos();

        if (GetApplicationGlobals()->doMenuPaste) {
            // TODO: Maybe use this: auto viewCenter = ImGui::GetMainViewport()->GetCenter();
            ImVec2 scrSize = ed::GetScreenSize();
            curMousePos.x += (500 + (scrSize.x / 2));
            curMousePos.y += (scrSize.y / 2);
        }

        flowMan.StopAutoTick();
        std::unordered_map<uint64_t, uint64_t> nodeRemap;
        ed::ClearSelection();
        if (state.contains("nodes")) {
            for (auto &node : state["nodes"]) {
                auto name = node["name"].get<std::string>();
                auto oldId = node["id"].get<uint64_t>();
                uint64_t new_node_id = flowMan.CreateNewNodeInstance(name.c_str());
                nodeRemap[oldId] = new_node_id;
                FlowCV::NodeInfo ni;
                flowMan.GetNodeInfoById(new_node_id, ni);
                if (node.contains("params")) {
                    ni.node_ptr->SetState(node["params"].dump());
                }
                ed::SetNodePosition((ed::NodeId) new_node_id, ImVec2(curMousePos.x, curMousePos.y));
                ed::SelectNode((ed::NodeId) new_node_id, true);
            }
        }

        // Remap Connections
        if (state.contains("connections")) {
            for (auto &conn : state["connections"]) {
                auto fromId = conn["from_id"].get<uint64_t>();
                auto fromIdx = conn["from_idx"].get<uint64_t>();
                auto toId = conn["to_id"].get<uint64_t>();
                auto toIdx = conn["to_idx"].get<uint64_t>();
                bool foundConnFrom = false;
                bool foundConnTo = false;
                if (nodeRemap.find(fromId) != nodeRemap.end()) {
                    fromId = nodeRemap[fromId];
                    foundConnFrom = true;
                }
                if (nodeRemap.find(toId) != nodeRemap.end()) {
                    toId = nodeRemap[toId];;
                    foundConnTo = true;
                }
                if (foundConnFrom && foundConnTo)
                    flowMan.ConnectNodes(fromId, fromIdx, toId, toIdx);
            }
        }

        // Place Locations with current mouse position offset
        ImVec2 startOff(0, 0);
        if (state.contains("editor")) {
            startOff.x = state["editor"].at(0)["location"]["x"].get<float>();
            startOff.y = state["editor"].at(0)["location"]["y"].get<float>();
            ImVec2 coordTrans(curMousePos.x - startOff.x, curMousePos.y - startOff.y);
            for (auto &edt : state["editor"]) {
                auto oldEdId = edt["id"].get<uint64_t>();
                if (nodeRemap.find(oldEdId) != nodeRemap.end()) {
                    ImVec2 curPos(edt["location"]["x"].get<float>(), edt["location"]["y"].get<float>());
                    ed::SetNodePosition((ed::NodeId) nodeRemap[oldEdId], ImVec2(curPos.x + coordTrans.x, curPos.y + coordTrans.y));
                }
            }
        }
        flowMan.StartAutoTick();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

static bool IsPinLinked(uint64_t pinId, FlowCV::FlowCV_Manager& flowMan)
{
    if (pinId == 0)
        return false;

    for (int i = 0; i < flowMan.GetWireCount(); i++) {
        FlowCV::Wire w = flowMan.GetWireInfoFromIndex(i);
        if ((w.to.id + IN_OFFSET + w.to.index) == pinId ||
            (w.from.id + OUT_OFFSET + w.from.index) == pinId) {
            return true;
        }
    }

    return false;
}

DerivedPinInfo GetPinInfoFromId(uint64_t pinId)
{
    DerivedPinInfo pinInfo{};

    uint64_t idMod = pinId % 1000;
    pinInfo.nodeId = (pinId - idMod) + 1;
    if (idMod < 200) {
        pinInfo.pinNum = (int32_t)idMod - 100;
        pinInfo.isInput = true;
    }
    else if (idMod >= 200)
    {
        pinInfo.pinNum = (int32_t)idMod - 200;
        pinInfo.isInput = false;
    }

    return pinInfo;
}

bool Application_SetState(FlowCV::FlowCV_Manager& flowMan, nlohmann::json& state)
{
    auto edGlobals = GetEditorGlobals();
    flowMan.StopAutoTick();
    ed::SetCurrentEditor(edGlobals->g_Context);
    bool res = flowMan.SetState(state);

    if (state.contains("editor")) {
        for (const auto &node : state["editor"]) {
            auto id = node["id"].get<uint64_t>();
            ImVec2 nodePos;
            nodePos.x = node["location"]["x"];
            nodePos.y = node["location"]["y"];
            ed::SetNodePosition((ed::NodeId)id, nodePos);
        }
    }
    edGlobals->g_NewState = 0;

    flowMan.StartAutoTick();

    return res;
}

nlohmann::json Application_GetState(FlowCV::FlowCV_Manager& flowMan)
{
    nlohmann::json nodes;
    ed::SetCurrentEditor(GetEditorGlobals()->g_Context);
    for (int i = 0; i < flowMan.GetNodeCount(); i++) {
        nlohmann::json node;
        nlohmann::json loc;
        uint64_t id = flowMan.GetNodeIdFromIndex(i);
        node["id"] = id;
        auto nodePos = ed::GetNodePosition((ed::NodeId)id);
        loc["x"] = nodePos.x;
        loc["y"] = nodePos.y;
        node["location"] = loc;
        nodes.emplace_back(node);
    }

    return nodes;
}

ImU32 GetNodeColor(DSPatch::Category cat)
{
    switch (cat)
    {
        default:
        case DSPatch::Category::Category_Source:                return IM_COL32(100, 100, 100, 128);
        case DSPatch::Category::Category_Output:                return IM_COL32(191, 191, 0, 128);
        case DSPatch::Category::Category_Draw:                  return IM_COL32(2, 145, 153, 128);
        case DSPatch::Category::Category_Color:                 return IM_COL32(122, 168, 255, 128);
        case DSPatch::Category::Category_Filter:                return IM_COL32(153, 61, 99, 128);
        case DSPatch::Category::Category_Merge:                 return IM_COL32(76, 94, 198, 128);
        case DSPatch::Category::Category_Transform:             return IM_COL32(179, 76, 25, 128);
        case DSPatch::Category::Category_Views:                 return IM_COL32(76, 128, 51, 128);
        case DSPatch::Category::Category_Feature_Detection:     return IM_COL32(204, 128, 76, 128);
        case DSPatch::Category::Category_Object_Detection:      return IM_COL32(187, 119, 255, 128);
        case DSPatch::Category::Category_OpenVino:              return IM_COL32(74, 196, 202, 128);
        case DSPatch::Category::Category_Utility:              return IM_COL32(111, 46, 158, 128);
        case DSPatch::Category::Category_Other:                 return IM_COL32(191, 191, 191, 128);
        case DSPatch::Category::Category_Experimental:          return IM_COL32(191, 191, 191, 128);
    }
}

ImColor GetIconColor(DSPatch::IoType type)
{
    switch (type)
    {
        default:
        case DSPatch::IoType::Io_Type_Unspecified:      return {255, 255, 255};
        case DSPatch::IoType::Io_Type_Bool:             return {220,  48,  48};
        case DSPatch::IoType::Io_Type_Int:              return { 68, 201, 156};
        case DSPatch::IoType::Io_Type_Float:            return {147, 226,  74};
        case DSPatch::IoType::Io_Type_String:           return {124,  21, 153};
        case DSPatch::IoType::Io_Type_CvMat:            return { 51, 150, 215};
        case DSPatch::IoType::Io_Type_JSON:             return {218,   0, 183};
        case DSPatch::IoType::Io_Type_Int_Array:        return {255,  48,  48};
    }
};

void DrawPinIcon(uint64_t pinId, DSPatch::IoType pinType, bool connected, int alpha)
{
    ax::Drawing::IconType iconType;
    DerivedPinInfo pin = GetPinInfoFromId(pinId);
    ImColor  color;
    color = GetIconColor(pinType);
    color.Value.w = (float)alpha / 255.0f;

    switch (pinType)
    {
        case DSPatch::IoType::Io_Type_Bool:         iconType = ax::Drawing::IconType::Flow;   break;
        case DSPatch::IoType::Io_Type_CvMat:        iconType = ax::Drawing::IconType::Circle; break;
        case DSPatch::IoType::Io_Type_Int:          iconType = ax::Drawing::IconType::Circle; break;
        case DSPatch::IoType::Io_Type_Float:        iconType = ax::Drawing::IconType::Circle; break;
        case DSPatch::IoType::Io_Type_String:       iconType = ax::Drawing::IconType::RoundSquare; break;
        case DSPatch::IoType::Io_Type_JSON:         iconType = ax::Drawing::IconType::Square; break;
        case DSPatch::IoType::Io_Type_Int_Array:    iconType = ax::Drawing::IconType::Grid; break;
        case DSPatch::IoType::Io_Type_Unspecified:  iconType = ax::Drawing::IconType::Diamond; break;
        default:
            ax::Drawing::IconType::Circle;
    }
    ImVec2 nSize = ed::GetNodeSize(pin.nodeId);
    ImVec2 nPos = ed::GetNodePosition(pin.nodeId);
    ImRect nodeSize = ImRect(ImVec2(nPos.x, nPos.y), ImVec2(nPos.x + nSize.x, nPos.y + nSize.y));
    ax::Widgets::Icon(ImVec2(24, 24), nodeSize, iconType, connected, color, ImColor(32, 32, 32, alpha), pin.isInput);
};

void Application_Frame(FlowCV::FlowCV_Manager& flowMan, const AppSettings &settings)
{
    static char nodeSearch[64] = "";
    static std::chrono::steady_clock::time_point currentTime;
    static std::chrono::steady_clock::time_point lastTime;
    auto edGlobals = GetEditorGlobals();
    auto appGlobals = GetApplicationGlobals();

    auto& io = ImGui::GetIO();
    if (settings.showFPS)
        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ed::SetCurrentEditor(edGlobals->g_Context);

    // Check for State Changes
    if (appGlobals->firstLoad) {
        edGlobals->g_currentState.clear();
        edGlobals->g_currentState = flowMan.GetState();
        edGlobals->g_currentState["editor"] = Application_GetState(flowMan);
        edGlobals->g_lastState.clear();
        edGlobals->g_lastState = edGlobals->g_currentState;
        appGlobals->firstLoad = false;
    }
    else {
        if (io.MouseReleased[ImGuiMouseButton_Left]) {
            currentTime = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count();
            if (delta >= 1) {
                edGlobals->g_currentState.clear();
                edGlobals->g_currentState = flowMan.GetState();
                edGlobals->g_currentState["editor"] = Application_GetState(flowMan);
                if (edGlobals->g_currentState != edGlobals->g_lastState) {
                    appGlobals->stateHasChanged = true;
                    appGlobals->stateIndicatorOnce = true;
                    edGlobals->g_lastState.clear();
                    edGlobals->g_lastState = edGlobals->g_currentState;
                }
                lastTime = currentTime;
            }
        }
    }

    // Start interaction with editor.
    ed::Begin("Node Editor", ImVec2(0.0, 0.0f));

    // Handle Cut, Copy and Paste
    if (!edGlobals->g_FirstFrame) {
        if (ed::AcceptCopy() || appGlobals->doMenuCopy) {
            CopySelectedNodes(flowMan);
            appGlobals->doMenuCopy = false;
        } else if (ed::AcceptCut() || appGlobals->doMenuCut) {
            CutSelectedNodes(flowMan);
            appGlobals->doMenuCut = false;
        } else if (ed::AcceptPaste() || appGlobals->doMenuPaste) {
            PasteNodes(flowMan);
            appGlobals->doMenuPaste = false;
        }
    }

    // Draw Nodes
    for (int i = 0; i < flowMan.GetNodeCount(); i++) {
        FlowCV::NodeInfo ni;
        flowMan.GetNodeInfoByIndex(i, ni);
        // Set Style
        ed::PushStyleVar(ed::StyleVar_NodeRounding, 1.0f);
        ed::PushStyleVar(ed::StyleVar_LinkStrength, 145.0f);
        ed::PushStyleVar(ed::StyleVar_NodeBorderWidth, 1.0f);
        ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBorder, ImVec4(1.0f, 1.0f, 1.0f, 0.3764f));
        if (ni.node_ptr->IsEnabled()) {
            ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, ImVec4(0.125f, 0.125f, 0.125f, 0.784f));
        }
        else {
            ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, ImVec4(0.125f, 0.125f, 0.125f, 0.45f));
        }
        ed::BeginNode((ed::NodeId)ni.id);
        ImGui::Text("%s_%i", ni.desc.name.c_str(), ni.node_ptr->GetInstanceCount());
        ImGui::Dummy(ImVec2(0, 8));
        ImGuiEx_BeginColumn();
        for (int j = 0; j < ni.desc.input_count; j++) {
            ed::BeginPin((ed::PinId)(ni.id + IN_OFFSET + j), ed::PinKind::Input);
                auto alpha = ImGui::GetStyle().Alpha;
                DSPatch::IoType pinType = ni.node_ptr->GetInputType(j);
                DrawPinIcon((ni.id + IN_OFFSET + j), pinType, IsPinLinked((ni.id + IN_OFFSET + j), flowMan), (int)(alpha * 255));
                ImGui::SameLine();
                ImGui::Text("%s", ni.node_ptr->GetInputName(j).c_str());
            ed::EndPin();
        }

        ImGuiEx_NextColumn();
        auto cursorPosOutPinStart = ImGui::GetCursorPos();
        size_t longestOutName = 0;
        for (int j = 0; j < ni.desc.output_count; j++) {
            if (ni.node_ptr->GetOutputName(j).size() > longestOutName)
                longestOutName = ni.node_ptr->GetOutputName(j).size();
        }
        if (longestOutName < 6)
            longestOutName = 6;
        for (int j = 0; j < ni.desc.output_count; j++) {
            auto cursorPos = ImGui::GetCursorPos();
            cursorPos.x = cursorPosOutPinStart.x + (((float)ni.desc.name.size() + 4.0f) * 1.75f);
            ImGui::SetCursorPosX(cursorPos.x);
            ed::BeginPin((ed::PinId)(ni.id + OUT_OFFSET + j), ed::PinKind::Output);
            auto alpha = ImGui::GetStyle().Alpha;
            std::string outLabel = ni.node_ptr->GetOutputName(j);
            if (outLabel.size() < longestOutName) {
                std::string padding;
                for (int k = 0; k < longestOutName - outLabel.size(); k++)
                    padding += " ";
                outLabel = padding + outLabel;
            }
            ImGui::Text("%s", outLabel.c_str());
            ImGui::SameLine();
            DSPatch::IoType pinType = ni.node_ptr->GetOutputType(j);
            DrawPinIcon((ni.id + OUT_OFFSET + j), pinType, IsPinLinked((ni.id + OUT_OFFSET + j), flowMan), (int)(alpha * 255));
            ed::EndPin();
        }
        ImGuiEx_EndColumn();

        ed::EndNode();
        auto drawList = ImGui::GetWindowDrawList();
        ImRect itemRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImRect baseRect = itemRect;

        // Now That We Have The Node Defined, Add Node Header Color Area
        itemRect.Max.y = itemRect.Min.y + 25;
        drawList->AddRectFilled(itemRect.GetTL(), itemRect.GetBR(),
                                GetNodeColor(ni.desc.category), 1.0f);
        drawList->AddLine(ImVec2(itemRect.Min.x, itemRect.Max.y), ImVec2(itemRect.Max.x - 1, itemRect.Max.y),
                          IM_COL32(132, 132, 132, 200), 1.0f);

        // If Disabled Draw an X over the node
        if (!ni.node_ptr->IsEnabled()) {
            drawList->AddLine(baseRect.Min, baseRect.Max, IM_COL32(227, 126, 18, 255), 1.0f);
            drawList->AddLine(ImVec2(baseRect.Min.x, baseRect.Max.y), ImVec2(baseRect.Max.x, baseRect.Min.y), IM_COL32(227, 126, 18, 255), 1.0f);
        }

        ed::PopStyleVar(3);
        ed::PopStyleColor(2);
    }

    // Submit Links
    for (int i = 0; i < flowMan.GetWireCount(); i++) {
        FlowCV::Wire w = flowMan.GetWireInfoFromIndex(i);
        FlowCV::NodeInfo ni;
        if (flowMan.GetNodeInfoById(w.from.id, ni)) {
            ed::Link((ed::LinkId) w.id, (ed::PinId) (w.from.id + OUT_OFFSET + w.from.index),
                     (ed::PinId) (w.to.id + IN_OFFSET + w.to.index),
                     GetIconColor(ni.node_ptr->GetOutputType((int)w.from.index)));
        }
    }
    //
    // 2) Handle interactions
    //
    // Handle Keyboard Input
    static bool setTabFocusOnce = false;
    if (appGlobals->allowEditorKeys && ImGui::GetActiveID() == 0 && ImGui::GetHoveredID() != 0) { // Fix to avoid shorcut keys triggering in other controls
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
            if (ImGui::IsKeyPressed(i)) {
                if (!edGlobals->g_ShowTabSearchWin) {
                    if (i == 'F') {
                        if (ed::GetSelectedObjectCount() > 0) {
                            ed::NavigateToSelection(true);
                        } else
                            ed::NavigateToContent();
                    } else if (i == 'D') {
                        std::vector<ed::NodeId> selectedNodes;
                        selectedNodes.resize(ed::GetSelectedObjectCount());
                        int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
                        for (const auto &node: selectedNodes) {
                            FlowCV::NodeInfo ni;
                            if (flowMan.GetNodeInfoById(node.Get(), ni)) {
                                bool curEnabled = ni.node_ptr->IsEnabled();
                                ni.node_ptr->SetEnabled(!curEnabled);
                            }
                        }
                    } else if (i == 'S') {
                        if (io.KeyCtrl) {
                            appGlobals->saveFlow = true;
                        } else {
                            for (int j = 0; j < flowMan.GetWireCount(); j++)
                                ed::Flow(flowMan.GetWireIdFromIndex(j));
                        }
                    } else if (i == 'N') {
                        if (io.KeyCtrl) {
                            appGlobals->newFlow = true;
                        }
                    } else if (i == 'L') {
                        if (io.KeyCtrl) {
                            appGlobals->showLoadDialog = true;
                        }
                    }
                }
                if (i == 258) {
                    edGlobals->g_NewNodePos = ImGui::GetMousePos();
                    nodeSearch[0] = '\0';
                    ImGui::OpenPopup("Find New Node");
                    edGlobals->g_ShowTabSearchWin = true;
                    setTabFocusOnce = true;
                }
            }
        }
    }

    auto dcId = ed::GetDoubleClickedNode();
    if (dcId) {
        int idx = flowMan.GetNodeIndexFromId(dcId.Get());
        flowMan.SetShowUI(idx, true);
        appGlobals->selectedId = dcId.Get();
    }

    auto openPopupPosition = ImGui::GetMousePos();
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&edGlobals->contextNodeId))
        ImGui::OpenPopup("Node Info");
    else if (ed::ShowBackgroundContextMenu()) {
        ImGui::OpenPopup("Create New Node");
        edGlobals->newNodeLinkPin = 0;
    }

    if (ImGui::BeginPopup("Node Info")) {
        FlowCV::NodeInfo ni;
        flowMan.GetNodeInfoById(edGlobals->contextNodeId.Get(), ni);

        ImGui::TextUnformatted("Node Info");
        ImGui::Separator();
        if (ni.id != 0)
        {
            auto categories = DSPatch::getCategories();
            ImGui::Text("ID: %#010llx", ni.id);
            ImGui::Text("Name: %s", ni.desc.name.c_str());
            ImGui::Text("Type: %s", categories.at(ni.desc.category));
            ImGui::Text("Inputs: %d", (int)ni.desc.input_count);
            ImGui::Text("Outputs: %d", (int)ni.desc.output_count);
            ImGui::Text("Author: %s", ni.desc.author.c_str());
            ImGui::Text("Version: %s", ni.desc.version.c_str());
        }
        else
            ImGui::Text("Unknown node: %p", edGlobals->contextNodeId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteNode(edGlobals->contextNodeId);
        ImGui::EndPopup();
    }

    auto viewCenter = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(viewCenter, ImGuiCond_Appearing);
    ImGui::SetNextWindowContentSize(ImVec2(380, 220));
    if (ImGui::BeginPopup("Find New Node")) {

        if (setTabFocusOnce) {
            ImGui::SetKeyboardFocusHere(0);
            setTabFocusOnce = false;
        }
        ImGui::InputText("Create/Find Node", nodeSearch, IM_ARRAYSIZE(nodeSearch), ImGuiInputTextFlags_AutoSelectAll);
        ImGui::Separator();
        ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int i = 0; i < flowMan.internal_node_manager_->NodeCount(); i++) {
            FlowCV::NodeDescription intNodeDesc;
            if (flowMan.internal_node_manager_->GetNodeDescription(i, intNodeDesc)) {
                if (FindStringCaseInsensitive(intNodeDesc.name, nodeSearch)) {
                    if (ImGui::MenuItem(intNodeDesc.name.c_str())) {
                        uint64_t newNodeId = flowMan.CreateNewNodeInstance(intNodeDesc.name.c_str());
                        ed::SetNodePosition(newNodeId, edGlobals->g_NewNodePos - ImVec2(60, 32));
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }
        for (int i = 0; i < flowMan.plugin_manager_->PluginCount(); i++) {
            FlowCV::NodeDescription plgNodeDesc;
            if (flowMan.plugin_manager_->GetPluginDescription(i, plgNodeDesc)) {
                if (FindStringCaseInsensitive(plgNodeDesc.name, nodeSearch)) {
                    if (ImGui::MenuItem(plgNodeDesc.name.c_str())) {
                        uint64_t newNodeId = flowMan.CreateNewNodeInstance(plgNodeDesc.name.c_str());
                        ed::SetNodePosition(newNodeId, edGlobals->g_NewNodePos - ImVec2(60, 32));
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
    else {
        edGlobals->g_ShowTabSearchWin = false;
    }


    if (ImGui::BeginPopup("Create New Node")) {
        if (edGlobals->g_SetNewNodePos) {
            edGlobals->g_NewNodePos = openPopupPosition;
            edGlobals->g_SetNewNodePos = false;
        }
        auto categories = DSPatch::getCategories();
        for (const auto &cat : categories) {
            if (ImGui::BeginMenu(cat.second)) {
                for (int i = 0; i < flowMan.internal_node_manager_->NodeCount(); i++) {
                    FlowCV::NodeDescription intNodeDesc;
                    if (flowMan.internal_node_manager_->GetNodeDescription(i, intNodeDesc)) {
                        if (intNodeDesc.category == cat.first) {
                            if (ImGui::MenuItem(intNodeDesc.name.c_str())) {
                                uint64_t newNodeId = flowMan.CreateNewNodeInstance(intNodeDesc.name.c_str());
                                ed::SetNodePosition(newNodeId, edGlobals->g_NewNodePos - ImVec2(60, 32));
                                if (edGlobals->createNewNode && edGlobals->newNodeLinkPin.Get() != 0) {
                                    DerivedPinInfo pi = GetPinInfoFromId(edGlobals->newNodeLinkPin.Get());
                                    FlowCV::NodeInfo pi_ni, ni;
                                    flowMan.GetNodeInfoById(pi.nodeId, pi_ni);
                                    flowMan.GetNodeInfoById(newNodeId, ni);
                                    if (pi.isInput) {
                                        if (ni.desc.output_count > 0) {
                                            if (ni.node_ptr->GetOutputType(0) == pi_ni.node_ptr->GetInputType(pi.pinNum))
                                                flowMan.ConnectNodes(ni.id, 0, pi.nodeId, pi.pinNum);
                                        }
                                    } else {
                                        if (ni.desc.input_count > 0) {
                                            if (pi_ni.node_ptr->GetOutputType(pi.pinNum) == ni.node_ptr->GetInputType(0))
                                                flowMan.ConnectNodes(pi.nodeId, pi.pinNum, ni.id, 0);
                                        }
                                    }
                                    edGlobals->createNewNode = false;
                                    edGlobals->newNodeLinkPin = 0;
                                }
                                edGlobals->g_SetNewNodePos = true;
                            }
                        }
                    }
                }
                for (int i = 0; i < flowMan.plugin_manager_->PluginCount(); i++) {
                    FlowCV::NodeDescription plgNodeDesc;
                    if (flowMan.plugin_manager_->GetPluginDescription(i, plgNodeDesc)) {
                        if (plgNodeDesc.category == cat.first) {
                            if (ImGui::MenuItem(plgNodeDesc.name.c_str())) {
                                uint64_t newNodeId = flowMan.CreateNewNodeInstance(plgNodeDesc.name.c_str());
                                ed::SetNodePosition(newNodeId, edGlobals->g_NewNodePos - ImVec2(60, 32));
                                if (edGlobals->createNewNode && edGlobals->newNodeLinkPin.Get() != 0) {
                                    DerivedPinInfo pi = GetPinInfoFromId(edGlobals->newNodeLinkPin.Get());
                                    FlowCV::NodeInfo pi_ni, ni;
                                    flowMan.GetNodeInfoById(pi.nodeId, pi_ni);
                                    flowMan.GetNodeInfoById(newNodeId, ni);
                                    if (pi.isInput) {
                                        if (ni.desc.output_count > 0) {
                                            if (ni.node_ptr->GetOutputType(0) == pi_ni.node_ptr->GetInputType(pi.pinNum))
                                                flowMan.ConnectNodes(ni.id, 0, pi.nodeId, pi.pinNum);
                                        }
                                    } else {
                                        if (ni.desc.input_count > 0) {
                                            if (pi_ni.node_ptr->GetOutputType(pi.pinNum) == ni.node_ptr->GetInputType(0))
                                                flowMan.ConnectNodes(pi.nodeId, pi.pinNum, ni.id, 0);
                                        }
                                    }
                                    edGlobals->createNewNode = false;
                                    edGlobals->newNodeLinkPin = 0;
                                }
                                edGlobals->g_SetNewNodePos = true;
                            }
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Show Flow", "S")) {
            for (int i = 0; i < flowMan.GetNodeCount(); i++)
                ed::Flow(flowMan.GetWireIdFromIndex(i));
        }

        ImGui::EndPopup();
    }
    else
        edGlobals->g_SetNewNodePos = true;

    ed::Resume();

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        auto showLabel = [](const char* label, ImColor color)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
            auto size = ImGui::CalcTextSize(label);

            auto padding = ImGui::GetStyle().FramePadding;
            auto spacing = ImGui::GetStyle().ItemSpacing;

            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

            auto rectMin = ImGui::GetCursorScreenPos() - padding;
            auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
            ImGui::TextUnformatted(label);
        };

        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
                DerivedPinInfo inputInfo = GetPinInfoFromId(inputPinId.Get());
                DerivedPinInfo outputInfo = GetPinInfoFromId(outputPinId.Get());

                if (!inputInfo.isInput) {
                    std::swap(inputPinId, outputPinId);
                    inputInfo = GetPinInfoFromId(inputPinId.Get());
                    outputInfo = GetPinInfoFromId(outputPinId.Get());
                }
                FlowCV::NodeInfo ni1, ni2;
                flowMan.GetNodeInfoById(inputInfo.nodeId, ni1);
                flowMan.GetNodeInfoById(outputInfo.nodeId, ni2);

                if (inputInfo.nodeId == outputInfo.nodeId) {
                    showLabel("x Not Allowed", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else if (inputPinId.Get() == outputPinId.Get()) {
                    showLabel("x Not Allowed", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else if (inputInfo.isInput && outputInfo.isInput) {
                    showLabel("x Not Allowed", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else if (!inputInfo.isInput && !outputInfo.isInput) {
                    showLabel("x Not Allowed", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else if (ni1.node_ptr->GetInputType(inputInfo.pinNum) != ni2.node_ptr->GetOutputType(outputInfo.pinNum)) {
                    showLabel("x Incompatible", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else {
                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                    if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
                        bool alreadyConnected = false;
                        bool inputAlreadyConnected = false;
                        int foundIdx = 0;
                        int idx = 0;
                        for (int i = 0; i < flowMan.GetWireCount(); i++) {
                            FlowCV::Wire w = flowMan.GetWireInfoFromIndex(i);
                            if (w.to.id == inputInfo.nodeId && w.from.id == outputInfo.nodeId) {
                                if (w.to.index == inputInfo.pinNum && outputInfo.pinNum == w.from.index) {
                                        alreadyConnected = true;
                                }
                            }
                            if (w.to.id == inputInfo.nodeId) {
                                if (w.to.index == inputInfo.pinNum) {
                                    inputAlreadyConnected = true;
                                    foundIdx = idx;
                                }
                            }
                            idx++;
                        }
                        if (!alreadyConnected) {
                            if (inputAlreadyConnected) {
                                flowMan.DisconnectNodeInput(inputInfo.nodeId, inputInfo.pinNum);
                            }
                            flowMan.ConnectNodes(outputInfo.nodeId, outputInfo.pinNum, inputInfo.nodeId, inputInfo.pinNum);

                            FlowCV::Wire w = flowMan.GetWireInfoFromIndex(flowMan.GetWireCount() - 1);
                            ed::Link((ed::LinkId) w.id, (ed::PinId) (w.from.id + OUT_OFFSET + w.from.index),
                                     (ed::PinId) (w.to.id + IN_OFFSET + w.to.index));
                        }
                    }
                }
            }
            else {
                showLabel("+ Create Node", ImColor(32, 45, 32, 180));
            }
        }

        ed::PinId pinId = 0;
        if (ed::QueryNewNode(&pinId))
        {
            DerivedPinInfo pinInfo = GetPinInfoFromId(pinId.Get());
            showLabel("+ Create Node", ImColor(32, 45, 32, 180));

            if (ed::AcceptNewItem())
            {
                edGlobals->createNewNode  = true;
                edGlobals->newNodeLinkPin = pinId;
                ed::Suspend();
                ImGui::OpenPopup("Create New Node");
                ed::Resume();
            }
        }
    }
    ed::EndCreate();

    if (ed::BeginDelete())
    {
        //flowMan.StopAutoTick();
        // There may be many links marked for deletion, let's loop over them.
        ed::LinkId deletedLinkId;
        ed::NodeId deletedNode;
        bool deleteLinkOnly = true;
        while (ed::QueryDeletedNode(&deletedNode))
        {
            if (ed::AcceptDeletedItem()) {
                flowMan.RemoveNodeInstance(deletedNode.Get());
                deleteLinkOnly = false;
            }
        }
        if (deleteLinkOnly) {
            while (ed::QueryDeletedLink(&deletedLinkId)) {
                // If you agree that link can be deleted, accept deletion.
                if (ed::AcceptDeletedItem()) {
                    // Then remove link from your data.
                    int idx = 0;
                    flowMan.RemoveWireById(deletedLinkId.Get());
                }
            }
        }
        //flowMan.StartAutoTick();
    }
    ed::EndDelete(); // Wrap up deletion action

    // End of interaction with editor.
    ed::End();

    if (edGlobals->g_NewState == 1) {
        ed::NavigateToContent();
        edGlobals->g_NewState = 3;
    } else if (edGlobals->g_NewState < 1) {
        edGlobals->g_NewState++;
    }

    ed::SetCurrentEditor(nullptr);

    edGlobals->g_FirstFrame = false;
}

