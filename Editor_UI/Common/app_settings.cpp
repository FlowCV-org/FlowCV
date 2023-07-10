//
// Common App Setting Code
//

#include "app_settings.h"

#include "FlowLogger.hpp"

void ApplicationLoadSettings(AppSettings &settings)
{
    if (std::filesystem::exists(settings.configPath)) {
        try {
            std::ifstream i(settings.configPath);
            nlohmann::json j;
            i >> j;
            i.close();
            if (j.contains("ext_plugin_dir")) {
                for (const auto &path : j["ext_plugin_dir"]) {
                    settings.extPluginDir.emplace_back(path.get<std::string>());
                }
            }
            if (j.contains("recent_size"))
                settings.recentListSize = j["recent_size"].get<int>();
            if (j.contains("recent_files")) {
                for (const auto &path : j["recent_files"]) {
                    settings.recentFiles.emplace_back(path.get<std::string>());
                }
            }
            if (j.contains("use_vsync"))
                settings.useVSync = j["use_vsync"].get<bool>();
            if (j.contains("show_fps"))
                settings.showFPS = j["show_fps"].get<bool>();
            if (j.contains("buffer_count"))
                settings.flowBufferCount = j["buffer_count"].get<int>();
        }
        catch (const std::exception &e) {
            LOG_ERROR("Error Loading Application Settings");
            std::cerr << e.what();
        }
    }

    if (settings.recentFiles.size() > settings.recentListSize) {
        int amtToRemove = settings.recentFiles.size() - settings.recentListSize;
        for (int i = 0; i < amtToRemove; i++)
            settings.recentFiles.pop_front();
    }
}

void ApplicationSaveSettings(const AppSettings &settings)
{
    nlohmann::json j;

    if (!settings.extPluginDir.empty())
        j["ext_plugin_dir"] = settings.extPluginDir;

    j["recent_size"] = settings.recentListSize;
    if (!settings.recentFiles.empty())
        j["recent_files"] = settings.recentFiles;

    if (settings.showFPS)
        j["show_fps"] = settings.showFPS;

    if (settings.useVSync)
        j["use_vsync"] = settings.useVSync;

    if (settings.flowBufferCount > 1)
        j["buffer_count"] = settings.flowBufferCount;

    if (!j.empty()) {
        std::ofstream o(settings.configPath);
        o << std::setw(4) << j << std::endl;
        o.close();
    }
}
