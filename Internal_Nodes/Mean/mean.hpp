//
// Plugin Mean
//

#ifndef FLOWCV_PLUGIN_MEAN_HPP_
#define FLOWCV_PLUGIN_MEAN_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Mean final : public Component
{
  public:
    Mean();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    float scale_;
    bool calc_mean_;
    bool calc_std_dev_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_MEAN_HPP_
