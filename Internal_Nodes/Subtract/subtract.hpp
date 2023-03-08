//
// Plugin Subtract
//

#ifndef FLOWCV_PLUGIN_SUBTRACT_HPP_
#define FLOWCV_PLUGIN_SUBTRACT_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Subtract final : public Component
{
  public:
    Subtract();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int mask_mode_;
    bool has_mask_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_SUBTRACT_HPP_
