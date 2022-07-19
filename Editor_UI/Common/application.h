#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_wrapper.hpp"
#include <app_settings.h>
#include <DSPatch.h>
#include <FlowCV_Manager.hpp>
#include <FlowCV_Types.hpp>
#include "json.hpp"
#include <deque>
#include "node_editor.h"

struct ApplicationGlobals {
    uint64_t selectedId{};
    bool showLoadDialog{};
    bool showSaveDialog{};
    bool newFlow{};
    bool saveFlow{};
    bool firstLoad{};
    bool stateHasChanged{};
    bool stateIndicatorOnce{};
    bool allowEditorKeys{};
    bool doMenuCut{};
    bool doMenuCopy{};
    bool doMenuPaste{};
};

ApplicationGlobals* GetApplicationGlobals();
const char* Application_GetName();
void Application_Initialize(std::string& appPath);
void Application_Finalize();
void Application_Frame(FlowCV::FlowCV_Manager& flowMan, const AppSettings &settings);
nlohmann::json Application_GetState(FlowCV::FlowCV_Manager& flowMan);
bool Application_SetState(FlowCV::FlowCV_Manager& flowMan, nlohmann::json& state);