//
// Headless (No GUI) Processing Engine
//

#include <iostream>
#include <csignal>
#include <app_settings.h>
#include <chrono>
#include <thread>
#include <tclap/CmdLine.h>
#include <filesystem>
#include <FlowCV_Manager.hpp>
#ifdef __linux__
#include <climits>
#include <unistd.h>
#include <libgen.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "FlowLogger.hpp"

#define APP_VERSION "0.1.3"

using namespace TCLAP;
using namespace std;

unsigned int g_bTerminate = 0;

void SignalHandler(int iSignal)
{
    if ((iSignal == SIGINT) || (iSignal == SIGTERM)) {
        g_bTerminate = 1;
    }
}

int Init_Signal()
{
    if (signal(SIGINT, SignalHandler) == SIG_ERR) {
        return -1;
    }

    if (signal(SIGTERM, SignalHandler) == SIG_ERR) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::string appDir;
    std::string cfgDir;
    AppSettings appSettings;
    CmdLine cmd("FlowCV Processing Engine", ' ', APP_VERSION);
    ValueArg<std::string> flow_file_arg("f", "flow", "Flow File", true, "", "string");
    ValueArg<std::string> cfg_file_arg("c", "cfg", "Custom Config File", false, "", "string");
    cmd.add(flow_file_arg);
    cmd.add(cfg_file_arg);
    cmd.parse(argc, argv);

    LOG_INFO("\nFlowCV Processing Engine - v{}\n", APP_VERSION);

    FlowCV::FlowCV_Manager flowMan;

    // Load Settings
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
    getenv_s(&requiredSize, nullptr, 0, "APPDATA");
    if (requiredSize != 0) {
        char *libVar = new char[requiredSize];
        getenv_s(&requiredSize, libVar, requiredSize, "APPDATA");
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
    std::string configFile;

    if (cfg_file_arg.getValue().empty()) {
        configFile = cfgDir;
        configFile += std::filesystem::path::preferred_separator;
        configFile += "flowcv_editor.cfg";
    }
    else {
        configFile = cfg_file_arg.getValue();
    }

    appSettings.configPath = configFile;
    ApplicationLoadSettings(appSettings);

    // Get Main App Plugins
    std::string pluginDir = appDir;
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
    LOG_INFO("{} Plugin(s) Loaded", flowMan.plugin_manager_->PluginCount());

    LOG_INFO("Loading Flow File: {}", flow_file_arg.getValue());
    if (!flowMan.LoadState(flow_file_arg.getValue().c_str())) {
        LOG_ERROR("Error Loading Flow File");
        return EXIT_FAILURE;
    }
    LOG_INFO("Flow State Loaded, {} Nodes Loaded and Configured", flowMan.GetNodeCount());

    // Init Signal Handling (Cntrl-C, Cntrl-X to clean exit)
    Init_Signal();

    LOG_INFO("Flow Processing Started");
    // Start Multi-Threaded Flow Processing in Background
    flowMan.StartAutoTick();

    while (!g_bTerminate) {
        // You Can do other things here while the Circuit Flow is running, for now we'll just sleep
        this_thread::sleep_for(chrono::seconds(1));
    }

    // Stop Flow Before Going Out of Scope and Cleanup
    flowMan.StopAutoTick();

    LOG_INFO("Flow Processing Stopped\nExiting");

    return EXIT_SUCCESS;
}
