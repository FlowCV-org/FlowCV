//
// FlowCV Manager
//

#ifndef FLOWCV_MANAGER_HPP_
#define FLOWCV_MANAGER_HPP_
#include <fstream>
#include <iostream>
#include <iomanip>
#include <DSPatch.h>
#include "Internal_Node_Manager.hpp"
#include "Plugin_Manager.hpp"
#include "json.hpp"

namespace FlowCV
{

struct IoInfo
{
    uint64_t id = 0;
    uint32_t index = 0;
};

struct NodeInfo
{
    uint64_t id;
    bool showControlUI;
    std::shared_ptr<DSPatch::Component> node_ptr;
    std::vector<IoInfo> input_conn_map;
    NodeDescription desc;
};

struct Wire
{
    uint64_t id{};
    IoInfo from;
    IoInfo to;
};

class FlowCV_Manager
{
  public:
    FlowCV_Manager();
    ~FlowCV_Manager();
    uint64_t CreateNewNodeInstance(const char *name);
    void SetBufferCount(uint32_t num_buffers);
    int GetBufferCount();
    uint64_t GetNodeCount();
    bool GetNodeInfoByIndex(uint64_t index, NodeInfo &nInfo);
    bool GetNodeInfoById(uint64_t id, NodeInfo &nInfo);
    int GetNodeIndexFromId(uint64_t id);
    uint64_t GetNodeIdFromIndex(uint64_t index);
    nlohmann::json GetState();
    void NewState();
    bool SetState(nlohmann::json &state);
    bool LoadState(const char *filepath);
    bool SaveState(const char *filepath);
    bool NodeHasUI(uint64_t index, GuiInterfaceType interface);
    void SetShowUI(uint64_t index, bool show);
    bool *GetShowUiPtr(uint64_t index);
    void ProcessNodeUI(uint64_t index, void *context, GuiInterfaceType interface);
    bool ConnectNodes(uint64_t from_id, uint32_t from_out_idx, uint64_t to_id, uint32_t to_in_idx);
    bool DisconnectNodes(uint64_t from_id, uint32_t from_out_idx, uint64_t to_id, uint32_t to_in_idx);
    uint64_t GetWireCount();
    Wire GetWireInfoFromIndex(uint64_t index);
    uint64_t GetWireIdFromIndex(uint64_t index);
    void RemoveWireById(uint64_t id);
    void RemoveWireByIndex(uint64_t index);
    std::vector<Wire> GetNodeConnectionsFromId(uint64_t id);
    std::vector<Wire> GetNodeConnectionsFromIndex(uint64_t index);
    void CheckInstCountValue(NodeInfo &ni);
    bool DisconnectNodeInput(uint64_t node_id, uint32_t in_index);
    bool RemoveNodeInstance(uint64_t node_id);
    void Tick(DSPatch::Component::TickMode mode = DSPatch::Component::TickMode::Parallel);
    void StartAutoTick(DSPatch::Component::TickMode mode = DSPatch::Component::TickMode::Parallel);
    void StopAutoTick();

  protected:
    uint64_t AddNewNodeInstance(const char *name, bool ext = false, uint64_t id = 0);
    uint64_t GetNextId();

  private:
    uint64_t id_counter_;
    uint64_t wire_id_counter_;
    std::vector<NodeInfo> nodes_;
    std::vector<Wire> wiring_;
    std::shared_ptr<DSPatch::Circuit> circuit_;

  public:
    std::shared_ptr<PluginManager> plugin_manager_;
    std::shared_ptr<InternalNodeManager> internal_node_manager_;
};
}  // End Namespace FlowCV
#endif  // FLOWCV_MANAGER_HPP_
