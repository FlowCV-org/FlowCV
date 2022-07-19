//
// Plugin Template
//

#ifndef FLOWCV_PLUGIN_TEMPLATE_HPP_
#define FLOWCV_PLUGIN_TEMPLATE_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{
namespace internal
{
class PluginName;
}

class DLLEXPORT PluginName final : public Component
{
  public:
    PluginName();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    std::unique_ptr<internal::PluginName> p;

};

EXPORT_PLUGIN( PluginName )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_TEMPLATE_HPP_
