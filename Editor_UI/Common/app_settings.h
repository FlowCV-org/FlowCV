//
// Common App Setting Header
//

#ifndef FLOWCV_APP_SETTINGS_H_
#define FLOWCV_APP_SETTINGS_H_
#include <iostream>
#include <vector>
#include <deque>
#include <filesystem>
#include <json.hpp>
#include <fstream>

struct AppSettings
{
    std::string configPath;
    std::vector<std::string> extPluginDir;
    int recentListSize;
    std::deque<std::string> recentFiles;
    bool showFPS;
    bool useVSync;
    int flowBufferCount;
};


void ApplicationLoadSettings(AppSettings &settings);
void ApplicationSaveSettings(const AppSettings &settings);


#endif //FLOWCV_APP_SETTINGS_H_
