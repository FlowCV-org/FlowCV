//
// Plugin Colorcorrect
//

#ifndef FLOWCV_PLUGIN_COLORCORRECT_HPP_
#define FLOWCV_PLUGIN_COLORCORRECT_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Colorcorrect final : public Component
{
  public:
    Colorcorrect();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    void UpdateLUT();

  private:
    std::array<uint8_t, 256> lut_cache_;
    ImVec4 color_range_low_;
    ImVec4 color_range_high_;
    float brightness_;
    float contrast_;
    float gain_;
    float gamma_;
    float saturation_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_COLORCORRECT_HPP_
