//
// FlowCV Manager
//

#include "FlowCV_Manager.hpp"
#include "FlowLogger.hpp"

namespace FlowCV
{

FlowCV_Manager::FlowCV_Manager()
{
    id_counter_ = 1001;
    wire_id_counter_ = 500;
    circuit_ = std::make_shared<DSPatch::Circuit>();
    plugin_manager_ = std::make_shared<PluginManager>();
    internal_node_manager_ = std::make_shared<InternalNodeManager>();
}

FlowCV_Manager::~FlowCV_Manager()
{
    nodes_.clear();
    circuit_->RemoveAllComponents();
}

uint64_t FlowCV_Manager::GetNextId()
{
    uint64_t nextId = id_counter_;

    for (const auto &node : nodes_) {
        if (node.id >= nextId)
            nextId = node.id + 1000;
    }

    id_counter_ = nextId;

    return nextId;
}

uint64_t FlowCV_Manager::AddNewNodeInstance(const char *name, bool ext, uint64_t id)
{
    NodeInfo ni;
    uint64_t ret_id = 0;

    if (ext)
        ni.node_ptr = plugin_manager_->CreatePluginInstance(name);
    else
        ni.node_ptr = internal_node_manager_->CreateNodeInstance(name);

    if (ni.node_ptr != nullptr) {
        ni.showControlUI = false;
        ni.desc.name = name;
        ni.desc.category = ni.node_ptr->GetComponentCategory();
        ni.desc.author = ni.node_ptr->GetComponentAuthor();
        ni.desc.version = ni.node_ptr->GetComponentVersion();
        ni.desc.input_count = ni.node_ptr->GetInputCount();
        ni.desc.output_count = ni.node_ptr->GetOutputCount();
        ni.input_conn_map.resize(ni.desc.input_count);
        if (id == 0) {
            ni.id = GetNextId();
        }
        else {
            ni.id = id;
            // GetNextId();
        }

        int res = circuit_->AddComponent(ni.node_ptr);
        if (res != -1)
            ret_id = ni.id;

        // Check Instance Count in case we have a loaded state causing a duplicate number
        CheckInstCountValue(ni);

        nodes_.emplace_back(std::move(ni));
    }

    return ret_id;
}

uint64_t FlowCV_Manager::CreateNewNodeInstance(const char *name)
{

    if (plugin_manager_->HasPlugin(name)) {
        return AddNewNodeInstance(name, true);
    }
    else if (internal_node_manager_->HasNode(name)) {
        return AddNewNodeInstance(name, false);
    }

    return 0;
}

void FlowCV_Manager::SetBufferCount(uint32_t num_buffers)
{
    circuit_->SetBufferCount(num_buffers);
}

int FlowCV_Manager::GetBufferCount()
{
    return circuit_->GetBufferCount();
}

void FlowCV_Manager::CheckInstCountValue(NodeInfo &ni)
{
    int cur_num = ni.node_ptr->GetInstanceCount();
    for (const auto &node : nodes_) {
        if (ni.desc.name == node.desc.name && ni.id != node.id) {
            if (cur_num <= node.node_ptr->GetInstanceCount()) {
                cur_num = node.node_ptr->GetInstanceCount() + 1;
            }
        }
    }
    ni.node_ptr->SetInstanceCount(cur_num);
}

uint64_t FlowCV_Manager::GetNodeCount()
{
    return (uint64_t)nodes_.size();
}

bool FlowCV_Manager::GetNodeInfoByIndex(uint64_t index, NodeInfo &nInfo)
{

    if (index >= 0 && index < nodes_.size()) {
        nInfo.showControlUI = nodes_.at(index).showControlUI;
        nInfo.desc.name = nodes_.at(index).desc.name;
        nInfo.desc.category = nodes_.at(index).node_ptr->GetComponentCategory();
        nInfo.desc.author = nodes_.at(index).node_ptr->GetComponentAuthor();
        nInfo.desc.version = nodes_.at(index).node_ptr->GetComponentVersion();
        nInfo.desc.input_count = nodes_.at(index).desc.input_count;
        nInfo.desc.output_count = nodes_.at(index).desc.output_count;
        nInfo.id = nodes_.at(index).id;
        nInfo.input_conn_map.assign(nodes_.at(index).input_conn_map.begin(), nodes_.at(index).input_conn_map.end());
        nInfo.node_ptr = nodes_.at(index).node_ptr;
        return true;
    }

    return false;
}

bool FlowCV_Manager::GetNodeInfoById(uint64_t id, NodeInfo &nInfo)
{
    return GetNodeInfoByIndex(GetNodeIndexFromId(id), nInfo);
}

int FlowCV_Manager::GetNodeIndexFromId(uint64_t id)
{

    for (int i = 0; i < nodes_.size(); i++) {
        if (nodes_.at(i).id == id) {
            return i;
        }
    }
    return 0;
}

uint64_t FlowCV_Manager::GetNodeIdFromIndex(uint64_t index)
{
    if (index >= 0 && index < nodes_.size()) {
        return nodes_.at(index).id;
    }
    return 0;
}

nlohmann::json FlowCV_Manager::GetState()
{
    using namespace nlohmann;
    json state;
    json nodes;
    json connections;

    for (const auto &node : nodes_) {
        json n;
        n["name"] = node.desc.name;
        n["id"] = node.id;
        n["num"] = node.node_ptr->GetInstanceCount();
        n["enabled"] = node.node_ptr->IsEnabled();
        std::string state_str = node.node_ptr->GetState();
        if (!state_str.empty()) {
            json node_state = json::parse(state_str);
            n["params"] = node_state;
        }
        nodes.push_back(n);
    }
    state["nodes"] = nodes;

    for (const auto &wire : wiring_) {
        json w;
        w["from_id"] = wire.from.id;
        w["from_idx"] = wire.from.index;
        w["to_id"] = wire.to.id;
        w["to_idx"] = wire.to.index;
        connections.push_back(w);
    }
    state["connections"] = connections;

    return std::move(state);
}

bool FlowCV_Manager::SetState(nlohmann::json &state)
{
    bool res = true;

    try {

        NewState();

        if (state.contains("nodes")) {
            for (const auto &node : state["nodes"]) {
                auto id = node["id"].get<uint64_t>();
                auto name = node["name"].get<std::string>();
                auto inst_num = node["num"].get<uint32_t>();
                bool enabled = true;
                if (node.contains("enabled"))
                    enabled = node["enabled"].get<bool>();
                if (plugin_manager_->HasPlugin(name.c_str())) {
                    uint64_t new_node_id = AddNewNodeInstance(name.c_str(), true, id);
                    NodeInfo ni;
                    GetNodeInfoById(new_node_id, ni);
                    ni.node_ptr->SetInstanceCount(inst_num);
                    ni.node_ptr->SetEnabled(enabled);
                    CheckInstCountValue(ni);
                    LOG_DEBUG("Adding Node Instance (Plugin): {}, {}", name, id);
                    if (node.contains("params")) {
                        ni.node_ptr->SetState(node["params"].dump());
                    }
                }
                else {
                    if (internal_node_manager_->HasNode(name.c_str())) {
                        uint64_t new_node_id = AddNewNodeInstance(name.c_str(), false, id);
                        NodeInfo ni;
                        GetNodeInfoById(new_node_id, ni);
                        ni.node_ptr->SetInstanceCount(inst_num);
                        ni.node_ptr->SetEnabled(enabled);
                        CheckInstCountValue(ni);
                        LOG_DEBUG("Adding Node Instance (Plugin): {}, {}", name, id);
                        if (node.contains("params")) {
                            ni.node_ptr->SetState(node["params"].dump());
                        }
                    }
                    else {
                        LOG_DEBUG("Node Name: {} Not Found", name);
                        res = false;
                    }
                }
            }
        }

        if (state.contains("connections")) {
            for (const auto &conn : state["connections"]) {
                auto to_id = conn["to_id"].get<uint64_t>();
                auto to_idx = conn["to_idx"].get<uint32_t>();
                auto from_id = conn["from_id"].get<uint64_t>();
                auto from_idx = conn["from_idx"].get<uint32_t>();
                ConnectNodes(from_id, from_idx, to_id, to_idx);
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what();
        return false;
    }

    return res;
}

void FlowCV_Manager::NewState()
{
    circuit_->RemoveAllComponents();
    wiring_.clear();
    nodes_.clear();
}

bool FlowCV_Manager::LoadState(const char *filepath)
{
    try {
        std::fstream i(filepath);
        nlohmann::json state;
        i >> state;
        i.close();

        return SetState(state);
    }
    catch (const std::exception &e) {
        std::cerr << e.what();
    }

    return false;
}

bool FlowCV_Manager::SaveState(const char *filepath)
{
    try {
        nlohmann::json state = GetState();
        std::ofstream o(filepath);
        o << std::setw(4) << state << std::endl;
        o.close();
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << e.what();
    }

    return false;
}

bool FlowCV_Manager::NodeHasUI(uint64_t index, GuiInterfaceType interface)
{
    if (index >= 0 && index < nodes_.size()) {
        return nodes_.at(index).node_ptr->HasGui((int)interface);
    }

    return false;
}

bool *FlowCV_Manager::GetShowUiPtr(uint64_t index)
{
    if (index >= 0 && index < nodes_.size()) {
        return &nodes_.at(index).showControlUI;
    }

    return nullptr;
}

void FlowCV_Manager::SetShowUI(uint64_t index, bool show)
{
    if (index >= 0 && index < nodes_.size()) {
        nodes_.at(index).showControlUI = show;
    }
}

void FlowCV_Manager::ProcessNodeUI(uint64_t index, void *context, GuiInterfaceType interface)
{
    if (index >= 0 && index < nodes_.size()) {
        return nodes_.at(index).node_ptr->UpdateGui(context, (int)interface);
    }
}

bool FlowCV_Manager::ConnectNodes(uint64_t from_id, uint32_t from_out_idx, uint64_t to_id, uint32_t to_in_idx)
{
    bool res = false;

    uint64_t from_index = GetNodeIndexFromId(from_id);
    uint64_t to_index = GetNodeIndexFromId(to_id);

    if (from_id != 0 && to_id != 0) {
        res = circuit_->ConnectOutToIn(nodes_.at(from_index).node_ptr, from_out_idx, nodes_.at(to_index).node_ptr, to_in_idx);
        if (res) {
            nodes_.at(to_index).input_conn_map.at(to_in_idx).id = from_id;
            nodes_.at(to_index).input_conn_map.at(to_in_idx).index = from_out_idx;
            Wire w;
            w.from.id = from_id;
            w.from.index = from_out_idx;
            w.to.id = to_id;
            w.to.index = to_in_idx;
            w.id = wire_id_counter_;
            wire_id_counter_ += 500;
            wiring_.emplace_back(w);
        }
    }

    return res;
}

bool FlowCV_Manager::DisconnectNodes(uint64_t from_id, uint32_t from_out_idx, uint64_t to_id, uint32_t to_in_idx)
{
    for (int i = 0; i < wiring_.size(); i++) {
        if (wiring_.at(i).from.id == from_id && wiring_.at(i).from.index == from_out_idx && wiring_.at(i).to.id == to_id &&
            wiring_.at(i).to.index == to_in_idx) {
            wiring_.erase(wiring_.begin() + i);
            uint64_t node_idx = GetNodeIndexFromId(to_id);
            nodes_.at(node_idx).node_ptr->DisconnectInput((int)to_in_idx);
            return true;
        }
    }

    return false;
}

bool FlowCV_Manager::DisconnectNodeInput(uint64_t node_id, uint32_t in_index)
{
    int node_idx = GetNodeIndexFromId(node_id);
    NodeInfo ni;
    if (GetNodeInfoById(node_id, ni)) {
        if (node_id != 0 && in_index >= 0 && in_index < ni.desc.input_count) {
            auto it = wiring_.begin();
            while (it != wiring_.end()) {
                if (it->to.id == node_id && it->to.index == in_index) {
                    it = wiring_.erase(it);
                    circuit_->PauseAutoTick();
                    nodes_.at(node_idx).node_ptr->DisconnectInput((int)in_index);
                    circuit_->ResumeAutoTick();
                    return true;
                }
                else
                    ++it;
            }
        }
    }

    return false;
}

bool FlowCV_Manager::RemoveNodeInstance(uint64_t node_id)
{
    bool res = false;

    int node_idx = GetNodeIndexFromId(node_id);
    if (node_id != 0 && node_idx < nodes_.size()) {
        circuit_->DisconnectComponent(nodes_.at(node_idx).node_ptr);
        circuit_->RemoveComponent(nodes_.at(node_idx).node_ptr);
        // Find Wires
        auto it = wiring_.begin();
        while (it != wiring_.end()) {
            if (it->to.id == node_id || it->from.id == node_id) {
                it = wiring_.erase(it);
            }
            else
                ++it;
        }
        nodes_.erase(nodes_.begin() + node_idx);
        res = true;
    }

    return res;
}

void FlowCV_Manager::Tick(DSPatch::Component::TickMode mode)
{
    circuit_->Tick(mode);
}

void FlowCV_Manager::StartAutoTick(DSPatch::Component::TickMode mode)
{
    circuit_->StartAutoTick(mode);
}

void FlowCV_Manager::StopAutoTick()
{
    circuit_->StopAutoTick();
}

std::vector<Wire> FlowCV_Manager::GetNodeConnectionsFromIndex(uint64_t index)
{
    std::vector<Wire> connections;
    if (index >= 0 && index < nodes_.size()) {
        NodeInfo ni;
        if (GetNodeInfoByIndex(index, ni)) {
            for (const auto &wire : wiring_) {
                if (wire.to.id == ni.id)
                    connections.emplace_back(wire);

                if (wire.from.id == ni.id)
                    connections.emplace_back(wire);
            }
        }
    }

    return connections;
}

std::vector<Wire> FlowCV_Manager::GetNodeConnectionsFromId(uint64_t id)
{
    int node_index = GetNodeIndexFromId(id);

    return GetNodeConnectionsFromIndex(node_index);
}

uint64_t FlowCV_Manager::GetWireCount()
{
    return (uint64_t)wiring_.size();
}

Wire FlowCV_Manager::GetWireInfoFromIndex(uint64_t index)
{
    Wire w;

    if (index >= 0 && index < wiring_.size()) {
        w.id = wiring_.at(index).id;
        w.to.id = wiring_.at(index).to.id;
        w.to.index = wiring_.at(index).to.index;
        w.from.id = wiring_.at(index).from.id;
        w.from.index = wiring_.at(index).from.index;
    }

    return w;
}

uint64_t FlowCV_Manager::GetWireIdFromIndex(uint64_t index)
{
    if (index >= 0 && index < wiring_.size()) {
        return wiring_.at(index).id;
    }
    return 0;
}

void FlowCV_Manager::RemoveWireById(uint64_t id)
{
    for (auto &wire : wiring_) {
        if (wire.id == id) {
            DisconnectNodeInput(wire.to.id, wire.to.index);
            return;
        }
    }
}

void FlowCV_Manager::RemoveWireByIndex(uint64_t index)
{
    if (index >= 0 && index < wiring_.size()) {
        DisconnectNodeInput(wiring_.at(index).to.id, wiring_.at(index).to.index);
    }
}

}  // End Namespace FlowCV
