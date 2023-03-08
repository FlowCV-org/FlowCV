//
// Plugin Threshold
//

#ifndef FLOWCV_PLUGIN_THRESHOLD_HPP_
#define FLOWCV_PLUGIN_THRESHOLD_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Threshold final : public Component
{
  public:
    Threshold();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int thresh_method_;
    int thresh_amt_;
    int thresh_type_;
    int adapt_type_;
    int adapt_thresh_;
    int adapt_block_;
    ImVec4 hsv_low_;
    ImVec4 hsv_high_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_THRESHOLD_HPP_
