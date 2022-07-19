//
// FlowCV Plugin Manager Class
//

#ifndef FLOWCV_PLUGIN_MANAGER_HPP_
#define FLOWCV_PLUGIN_MANAGER_HPP_
#include <iostream>
#include <DSPatch.h>
#include "FlowCV_Types.hpp"

namespace FlowCV
{

struct PluginInfo {
    bool is_initialized = false;
    NodeDescription plugin_desc;
    DSPatch::Plugin *plugin_handle{};
};

class PluginManager {
  public:
    PluginManager() = default;
    ~PluginManager();
    void LoadPlugins(const char *plugin_path, bool recursive = true);
    void UnLoadPlugins();
    uint32_t PluginCount();
    bool GetPluginDescription(uint32_t index, NodeDescription &nodeDesc);
    bool GetPluginDescription(const char *name, NodeDescription &nodeDesc);
    std::shared_ptr<DSPatch::Component> CreatePluginInstance(const char *name);
    bool HasPlugin(const char *name);

  protected:
    void ScanDirForPlugins(const char *dir_path, bool recursive = true);

  private:
    std::string plugin_path_;
    std::vector<PluginInfo> plugins_;
};
} // End Namespace FlowCV
#endif //FLOWCV_PLUGIN_MANAGER_HPP_
