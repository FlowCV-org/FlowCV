//
// FlowCV Plugin Manager Class
//

#include "Plugin_Manager.hpp"
#include <filesystem>
#include <algorithm>

namespace FlowCV
{
PluginManager::~PluginManager()
{
    UnLoadPlugins();
}

static bool compareName(const PluginInfo &a, const PluginInfo &b)
{
    return a.plugin_desc.name < b.plugin_desc.name;
}

void PluginManager::ScanDirForPlugins(const char *dir_path, bool recursive)
{
    namespace fs = std::filesystem;
    const std::string ext = ".fp";

    for (const auto &entry : fs::directory_iterator(dir_path)) {
        if (entry.is_directory()) {
            if (recursive)
                ScanDirForPlugins(entry.path().string().c_str());
        }
        else if (entry.is_regular_file()) {
            std::string filename = entry.path().string();
            int extLen = ext.size();
            std::string ending = filename.substr(filename.size() - extLen, extLen);
            if (ending == ext) {
                std::cout << entry.path() << std::endl;
                PluginInfo pi;
                pi.plugin_handle = new DSPatch::Plugin(filename);
                if (pi.plugin_handle->IsLoaded()) {
                    std::shared_ptr<DSPatch::Component> plugin_instance = pi.plugin_handle->Create();
                    pi.plugin_desc.name = plugin_instance->GetComponentName();
                    pi.plugin_desc.category = plugin_instance->GetComponentCategory();
                    pi.plugin_desc.author = plugin_instance->GetComponentAuthor();
                    pi.plugin_desc.version = plugin_instance->GetComponentVersion();
                    pi.plugin_desc.input_count = plugin_instance->GetInputCount();
                    pi.plugin_desc.output_count = plugin_instance->GetOutputCount();
                    plugins_.emplace_back(pi);
                    plugin_instance.reset();
                }
            }
        }
    }

    // Sort Nodes
    std::sort(plugins_.begin(), plugins_.end(), compareName);
}

void PluginManager::LoadPlugins(const char *plugin_path, bool recursive)
{
    plugin_path_ = plugin_path;
    std::cout << "Looking for Plugins in: " << plugin_path << std::endl;
    ScanDirForPlugins(plugin_path_.c_str());
}

void PluginManager::UnLoadPlugins()
{
    if (!plugins_.empty()) {
        for (auto &p : plugins_) {
            delete p.plugin_handle;
        }
    }
}

uint32_t PluginManager::PluginCount()
{
    return plugins_.size();
}

bool PluginManager::GetPluginDescription(uint32_t index, NodeDescription &nodeDesc)
{

    if (index >= 0 && index < plugins_.size()) {
        nodeDesc.name = plugins_.at(index).plugin_desc.name;
        nodeDesc.category = plugins_.at(index).plugin_desc.category;
        nodeDesc.author = plugins_.at(index).plugin_desc.author;
        nodeDesc.version = plugins_.at(index).plugin_desc.version;
        nodeDesc.input_count = plugins_.at(index).plugin_desc.input_count;
        nodeDesc.output_count = plugins_.at(index).plugin_desc.output_count;
        return true;
    }

    return false;
}

bool PluginManager::GetPluginDescription(const char *name, NodeDescription &nodeDesc)
{

    for (auto const &p : plugins_) {
        if (p.plugin_desc.name == name) {
            nodeDesc.name = p.plugin_desc.name;
            nodeDesc.category = p.plugin_desc.category;
            nodeDesc.author = p.plugin_desc.author;
            nodeDesc.version = p.plugin_desc.version;
            nodeDesc.input_count = p.plugin_desc.input_count;
            nodeDesc.output_count = p.plugin_desc.output_count;
            return true;
        }
    }

    return false;
}

std::shared_ptr<DSPatch::Component> PluginManager::CreatePluginInstance(const char *name)
{
    for (auto const &p : plugins_) {
        if (p.plugin_desc.name == name) {
            std::shared_ptr<DSPatch::Component> pi = p.plugin_handle->Create();
            return pi;
        }
    }
    return nullptr;
}

bool PluginManager::HasPlugin(const char *name)
{
    for (auto const &p : plugins_) {
        if (p.plugin_desc.name == name) {
            return true;
        }
    }

    return false;
}

}  // End Namespace FlowCV
