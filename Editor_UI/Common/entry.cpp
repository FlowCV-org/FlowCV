#include <application.h>
#include <cstdio>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <ImGuiFileBrowser.h>
#include <tclap/CmdLine.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

using namespace TCLAP;

ApplicationGlobals* GetApplicationGlobals()
{
    static auto app_globals = new ApplicationGlobals;

    return app_globals;
}

void AddFileToRecent(AppSettings &settings, const std::string &filePath)
{
    settings.recentFiles.emplace_back(filePath);
    if (settings.recentFiles.size() > settings.recentListSize) {
        settings.recentFiles.pop_front();
    }
    ApplicationSaveSettings(settings);
}

void ApplicationAboutDialog(bool &dialogState)
{
    auto viewCenter = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(viewCenter, ImGuiCond_Appearing);
    ImGui::Begin("About FlowCV", &dialogState, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FlowCV %s", FLOWCV_EDITOR_VERSION_STR);
    ImGui::Separator();
    ImGui::Text("By Richard Wardlow");
    ImGui::Text("An OpenCV Dataflow Framework and Node Editor UI");
    ImGui::End();
}

void ApplicationSettingsDialog(AppSettings &settings, bool &windowState)
{
    static int plugin_path_select = -1;
    static bool showPathDialog = false;
    static imgui_addons::ImGuiFileBrowser file_dialog_plugins;

    auto viewCenter = ImGui::GetMainViewport()->GetCenter();
    viewCenter.x -= 300;
    viewCenter.y -= 300;
    ImGui::SetNextWindowPos(viewCenter, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 450), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Application Settings", &windowState, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Separator();
    ImGui::SetNextItemWidth(80);
    if (ImGui::InputInt("Recent File History Size", &settings.recentListSize)) {
        if (settings.recentListSize < 1)
            settings.recentListSize = 1;
    }
    ImGui::Separator();
    ImGui::SetNextItemWidth(80);
    if (ImGui::InputInt("Flow Buffer Count", &settings.flowBufferCount)) {
        if (settings.flowBufferCount < 1)
            settings.flowBufferCount = 1;
    }
    ImGui::Checkbox("Show FPS", &settings.showFPS);
    ImGui::Separator();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("ChildR", ImVec2(-1, 260), true, ImGuiWindowFlags_None);
    ImGui::Text("Extra Plugin Paths:");
    ImGui::ListBoxHeader("##PluginPaths", ImVec2(-1, 200));
    bool sel;
    for (int i = 0; i < settings.extPluginDir.size(); i++) {
        if (i == plugin_path_select)
            sel = ImGui::Selectable(settings.extPluginDir.at(i).c_str(), true);
        else
            sel = ImGui::Selectable(settings.extPluginDir.at(i).c_str(), false);

        if (sel) {
            std::cout << i << std::endl;
            plugin_path_select = i;
        }
    }
    ImGui::ListBoxFooter();
    ImGui::Separator();
    ImGui::Columns(2);
    if (ImGui::Button("Add Plugin Path", ImVec2(-FLT_MIN, 0.0f))) {
        showPathDialog = true;
    }
    ImGui::NextColumn();
    if (ImGui::Button("Remove Selected Path", ImVec2(-FLT_MIN, 0.0f))) {
        settings.extPluginDir.erase(settings.extPluginDir.begin() + plugin_path_select);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::Separator();
    ImVec2 curSize = ImGui::GetWindowSize();
    ImVec2 curPos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(-1, (curSize.y - curPos.y) - 50));
    ImGui::Dummy(ImVec2((curSize.x / 2) - 80, 50));
    ImGui::SameLine();
    if (ImGui::Button("Save and Close", ImVec2(140, 30))) {
        ApplicationSaveSettings(settings);
        ImGui::OpenPopup("Need to Restart");
    }

    if (showPathDialog)
        ImGui::OpenPopup("Select New Plugin Path");

    if(file_dialog_plugins.showFileDialog("Select New Plugin Path", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT, ImVec2(700, 310), "*.*", &showPathDialog)) {
        std::string newPath = file_dialog_plugins.selected_path;
        // Fix slash in case of Windows OS
        std::replace( newPath.begin(), newPath.end(), '/', (const char)std::filesystem::path::preferred_separator);
        settings.extPluginDir.emplace_back(newPath);
        showPathDialog = false;
    }
    if (ImGui::BeginPopupModal("Need to Restart", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You Need To Close The App and Restart It For New Plugin Paths To Take Effect");
        if (ImGui::Button("OK")) {
            windowState = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

std::string SaveFlowFile(ImGuiWrapper &imgui, FlowCV::FlowCV_Manager &flowMan, std::string filename)
{
    auto appGlobals = GetApplicationGlobals();
    nlohmann::json state = flowMan.GetState();
    state["editor"] = Application_GetState(flowMan);
    std::ofstream o(filename);
    o << std::setw(4) << state << std::endl;
    o.close();
    std::string appTitle = Application_GetName();
    appTitle += " - ";
    appTitle += filename;
    imgui.SetWindowTitle(appTitle.c_str());
    appGlobals->firstLoad = true;
    appGlobals->stateHasChanged = false;
    appGlobals->showSaveDialog = false;

    return appTitle;
}

int main(int argc, char *argv[])
{
    FlowCV::FlowCV_Manager flowMan;
    ImGuiWrapper imgui;
    std::string pluginDir;
    std::string appDir;
    std::string cfgDir;
    std::string configFile;
    auto appGlobals = GetApplicationGlobals();
    AppSettings appSettings;
    appSettings.recentListSize = 8;
    appSettings.flowBufferCount = 1;
    appSettings.showFPS = false;

    CmdLine cmd("FlowCV Node Editor", ' ', FLOWCV_EDITOR_VERSION_STR);
    ValueArg<std::string> cfg_file_arg("c", "cfg", "Default Config File Override", false, "", "string");
    cmd.add(cfg_file_arg);

    cmd.parse(argc, argv);

    // Load App Settings
#ifdef __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *path;
    if (count != -1) {
        path = dirname(result);
    }
    appDir = path;
    cfgDir = path;
#endif
#ifdef _WINDOWS
    appDir = std::filesystem::current_path().string();
    size_t requiredSize;
    getenv_s( &requiredSize, nullptr, 0, "APPDATA");
    if (requiredSize != 0) {
        char *libVar = new char[requiredSize];
        getenv_s( &requiredSize, libVar, requiredSize, "APPDATA" );
        cfgDir = libVar;
        cfgDir += std::filesystem::path::preferred_separator;
        cfgDir += "FlowCV";
        if (!std::filesystem::exists(cfgDir)) {
            std::filesystem::create_directories(cfgDir);
        }
        delete[] libVar;
    }
    else {
        cfgDir = appDir;
    }
#endif
#if __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::filesystem::path p = path;
        appDir = p.parent_path();
    }
    else
        appDir = std::filesystem::current_path().string();
    cfgDir = appDir;
#endif
    if (cfg_file_arg.getValue().empty()) {
        configFile = cfgDir;
        configFile += std::filesystem::path::preferred_separator;
        configFile += "flowcv_editor.cfg";
    }
    else
        configFile = cfg_file_arg.getValue();

    appSettings.configPath = configFile;
    ApplicationLoadSettings(appSettings);

    // Get Main App Plugins
    pluginDir = appDir;
    pluginDir += std::filesystem::path::preferred_separator;
    pluginDir += "Plugins";
    if (std::filesystem::exists(pluginDir))
        flowMan.plugin_manager_->LoadPlugins(pluginDir.c_str());

    // Get Extra Plugins
    if (!appSettings.extPluginDir.empty()) {
        for (const auto &path : appSettings.extPluginDir) {
            if (std::filesystem::exists(path))
                flowMan.plugin_manager_->LoadPlugins(path.c_str(), false);
        }
    }

    flowMan.CreateNewNodeInstance("Viewer");

    std::string appTitle = Application_GetName();
    imgui.Init(1280, 720, appTitle.c_str(), ImGuiConfigFlags_ViewportsEnable);
    ImGuiIO& io = ImGui::GetIO();
#ifdef __linux__ // Fix to make sure ini file goes into the app dir instead of working dir
    std::string imgui_ini = appDir;
    imgui_ini += std::filesystem::path::preferred_separator;
    imgui_ini += "imgui.ini";
    // copy std::string to char array since io.IniFilename doesn't work well with c_str() pointer to std::string
    char iniPath[PATH_MAX];
    sprintf(iniPath, "%s", imgui_ini.c_str());
    io.IniFilename = iniPath;
#endif
    bool initialized = false;
    bool showAboutDialog = false;
    bool showAppSettings = false;
    ImGuiID dock_main_id, dock_id_prop;
    ImGuiID dockspace_id;

    std::string errorMsg;
    imgui_addons::ImGuiFileBrowser file_dialog;

    Application_Initialize(appDir);

    if (appSettings.flowBufferCount > 1)
        flowMan.SetBufferCount(appSettings.flowBufferCount);
    flowMan.StartAutoTick();

    appGlobals->selectedId = 0;

    std::string currently_opened_flow_file;

    // Main loop
    bool closeOnceGood = false;
    while (!closeOnceGood)
    {
        ImGuiWrapper::PollEvents();
        ImGuiWrapper::NewFrame();

        // Set State Changed Indicator
        if (appGlobals->stateHasChanged) {
            if (appGlobals->stateIndicatorOnce) {
                if (!currently_opened_flow_file.empty()) {
                    appTitle = Application_GetName();
                    appTitle += " - *";
                    appTitle += currently_opened_flow_file;
                    imgui.SetWindowTitle(appTitle.c_str());
                }
                appGlobals->stateIndicatorOnce = false;
            }
        }

        // Handle Node Editor Operation Requests
        if (appGlobals->newFlow) {
            flowMan.NewState();
            appTitle = Application_GetName();
            imgui.SetWindowTitle(appTitle.c_str());
            currently_opened_flow_file = "";
            appGlobals->firstLoad = true;
            appGlobals->newFlow = false;
        }
        if (appGlobals->saveFlow) {
            if (!currently_opened_flow_file.empty()) {
                appTitle = SaveFlowFile(imgui, flowMan, currently_opened_flow_file);
            }
            else {
                appGlobals->showSaveDialog = true;
            }
            appGlobals->saveFlow = false;
        }

        // Add Main Menu Bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "CTRL+N"))
                {
                    flowMan.NewState();
                    appTitle = Application_GetName();
                    imgui.SetWindowTitle(appTitle.c_str());
                    currently_opened_flow_file = "";
                    appGlobals->firstLoad = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Load", "CTRL+L"))
                {
                    appGlobals->showLoadDialog = true;
                }
                if (ImGui::MenuItem("Save", "CTRL+S"))
                {
                    if (!currently_opened_flow_file.empty()) {
                        appTitle = SaveFlowFile(imgui, flowMan, currently_opened_flow_file);
                    }
                    else {
                        appGlobals->showSaveDialog = true;
                    }
                }
                if (ImGui::MenuItem("Save As..."))
                {
                    appGlobals->showSaveDialog = true;
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Load Recent")) {
                    for (auto path = appSettings.recentFiles.rbegin(); path != appSettings.recentFiles.rend(); ++path) {
                        if (ImGui::MenuItem(path->c_str())) {
                            flowMan.StopAutoTick();
                            appGlobals->firstLoad = true;
                            std::fstream i(*path);
                            currently_opened_flow_file = *path;
                            nlohmann::json state;
                            i >> state;
                            i.close();
                            bool res = Application_SetState(flowMan, state);
                            appTitle = Application_GetName();
                            appTitle += " - ";
                            appTitle += currently_opened_flow_file;
                            imgui.SetWindowTitle(appTitle.c_str());
                            appGlobals->stateHasChanged = false;
                            flowMan.StartAutoTick();
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "")) {
                    imgui.SetShouldClose(true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "CTRL+Z", false, false)) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {
                    appGlobals->doMenuCut = true;
                }
                if (ImGui::MenuItem("Copy", "CTRL+C")) {
                    appGlobals->doMenuCopy = true;
                }
                if (ImGui::MenuItem("Paste", "CTRL+V")) {
                    appGlobals->doMenuPaste = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Settings")) {
                    showAppSettings = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("Documentation", "")) {
                    std::string url = "http://docs.flowcv.org/editor/node_editor_start.html";
                    std::string op;
#ifdef _WIN32
                    op = std::string("start ").append(url);
#endif
#ifdef __linux__
                    op = std::string("xdg-open ").append(url);
#endif
                    system(op.c_str());
                }
                ImGui::Separator();
                ImGui::MenuItem("About", NULL, &showAboutDialog);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::BeginPopupModal("Error Dialog", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
            ImGui::Text("%s", errorMsg.c_str());
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        if (imgui.ShouldClose()) {
            if (appGlobals->stateHasChanged && !appGlobals->showSaveDialog) {
                if (!ImGui::IsPopupOpen("Unsaved Changes"))
                    ImGui::OpenPopup("Unsaved Changes");

                auto viewCenter = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(ImVec2(viewCenter.x - 140, viewCenter.y - 40));
                ImGui::SetNextWindowSize(ImVec2(280, 80));
                if (ImGui::BeginPopupModal("Unsaved Changes")) {
                    ImGui::TextUnformatted("Save Changes Before Exit?");
                    ImGui::Separator();
                    if (ImGui::Button("Save", ImVec2(120, 0))) {
                        if (!currently_opened_flow_file.empty()) {
                            SaveFlowFile(imgui, flowMan, currently_opened_flow_file);
                            appGlobals->stateHasChanged = false;
                        }
                        else {
                            appGlobals->showSaveDialog = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Quit", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                        appGlobals->stateHasChanged = false;
                        closeOnceGood = true;
                        flowMan.StopAutoTick();
                    }
                    ImGui::EndPopup();
                }
            } else if (!appGlobals->stateHasChanged && !appGlobals->showSaveDialog) {
                closeOnceGood = true;
                flowMan.StopAutoTick();
            }
        }

        if(appGlobals->showLoadDialog) {
            ImGui::OpenPopup("Open File");
            appGlobals->allowEditorKeys = false;
        }

        if(appGlobals->showSaveDialog) {
            ImGui::OpenPopup("Save As...");
            appGlobals->allowEditorKeys = false;
        }

        if(showAboutDialog)
            ApplicationAboutDialog(showAboutDialog);

        if (showAppSettings)
            ApplicationSettingsDialog(appSettings, showAppSettings);

        if(file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".flow", &appGlobals->showLoadDialog))
        {
            flowMan.StopAutoTick();
            appGlobals->firstLoad = true;
            printf("OPEN[%s]\n", file_dialog.selected_path.c_str());
            AddFileToRecent(appSettings, file_dialog.selected_path);
            std::fstream i(file_dialog.selected_path);
            currently_opened_flow_file = file_dialog.selected_path;
            nlohmann::json state;
            i >> state;
            i.close();
            bool res = Application_SetState(flowMan, state);
            appGlobals->showLoadDialog = false;
            if (!res) {
                std::cout << "Error!" << std::endl;
                errorMsg = "There Were Errors Loading FLow";
                ImGui::OpenPopup("Error Dialog");
            }
            appTitle = Application_GetName();
            appTitle += " - ";
            appTitle += currently_opened_flow_file;
            imgui.SetWindowTitle(appTitle.c_str());
            appGlobals->stateHasChanged = false;
            appGlobals->allowEditorKeys = true;
            flowMan.StartAutoTick();
        }

        if (file_dialog.showFileDialog("Save As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".flow", &appGlobals->showSaveDialog)) {
            flowMan.StopAutoTick();
            if (file_dialog.selected_path.find(".flow") == std::string::npos)
                file_dialog.selected_path += ".flow";
            printf("SAVE[%s]\n", file_dialog.selected_path.c_str());
            currently_opened_flow_file = file_dialog.selected_path;
            AddFileToRecent(appSettings, file_dialog.selected_path);
            appTitle = SaveFlowFile(imgui, flowMan, currently_opened_flow_file);
            appGlobals->allowEditorKeys = true;
            flowMan.StartAutoTick();
        }

        if (!appGlobals->showSaveDialog && !appGlobals->showLoadDialog)
            appGlobals->allowEditorKeys = true;

        ImGuiWrapper::StartDockSpace(true);
        dockspace_id = ImGui::GetID("InvisibleWindowDockSpace");
        if (!initialized)
        {
            initialized = true;

            dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
            auto ds = ImGui::DockBuilderGetNode(dock_main_id);
            if (!ds->IsSplitNode())
                dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
            else
                dock_id_prop = ImGui::GetWindowDockID();

            ImGui::DockBuilderFinish(dockspace_id);
        }

        // Process Nodes Controls UI
        ImGui::SetNextWindowDockID(dock_id_prop, ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties");
        for (int i = 0; i < flowMan.GetNodeCount(); i++) {
            if (flowMan.NodeHasUI(i, FlowCV::GuiInterfaceType_Controls)) {
                FlowCV::NodeInfo ni;
                 flowMan.GetNodeInfoByIndex(i, ni);
                if (ni.showControlUI) {
                    std::string param_name = ni.desc.name;
                    param_name += "_";
                    param_name += std::to_string(ni.node_ptr->GetInstanceCount());
                    bool *ctlOpen = flowMan.GetShowUiPtr(i);
                    if (ctlOpen != nullptr) {
                        if (appGlobals->selectedId == ni.id) {
                            ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(39, 154, 14, 255));
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(45, 52, 65, 255));
                        }
                        if (ImGui::CollapsingHeader(param_name.c_str(), ctlOpen, ImGuiTreeNodeFlags_DefaultOpen) ) {
                            flowMan.ProcessNodeUI(i, imgui.GetImGuiCurrentContext(), FlowCV::GuiInterfaceType_Controls);
                        }
                        ImGui::PopStyleColor();
                    }
                    ImGui::Separator();
                    ImGui::Separator();
                }
            }
        }
        ImGui::End();

        // Process Nodes Main UI
        for (int i = 0; i < flowMan.GetNodeCount(); i++) {
            if (flowMan.NodeHasUI(i, FlowCV::GuiInterfaceType_Main)) {
                FlowCV::NodeInfo ni;
                flowMan.GetNodeInfoByIndex(i, ni);
                ImGui::SetNextWindowDockID(dock_main_id, ImGuiCond_FirstUseEver);
                flowMan.ProcessNodeUI(i, imgui.GetImGuiCurrentContext(), FlowCV::GuiInterfaceType_Main);
            }
        }

        ImGui::SetNextWindowDockID(dock_main_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("Editor");
        Application_Frame(flowMan, appSettings);
        ImGui::End();

        ImGuiWrapper::FrameEnd();
        imgui.Update();
    }

    Application_Finalize();

    return 0;
}
