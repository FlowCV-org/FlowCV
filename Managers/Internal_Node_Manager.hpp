//
// FlowCV Internal Component Manager
//

#ifndef FLOWCV_INTERNAL_NODE_MANAGER_HPP_
#define FLOWCV_INTERNAL_NODE_MANAGER_HPP_
#include <iostream>
#include <DSPatch.h>
#include "FlowCV_Types.hpp"

namespace FlowCV {

class InternalNodeManager {
  public:
    InternalNodeManager();
    ~InternalNodeManager() = default;
    uint32_t NodeCount();
    bool GetNodeDescription(uint32_t index, NodeDescription &nodeDesc);
    bool GetNodeDescription(const char *name, NodeDescription &nodeDesc);
    std::shared_ptr<DSPatch::Component> CreateNodeInstance(const char *name);
    bool HasNode(const char *name);
  protected:

  private:
    std::vector<NodeDescription> node_list_;

};
} // End Namespace FlowCV
#endif //FLOWCV_INTERNAL_NODE_MANAGER_HPP_
