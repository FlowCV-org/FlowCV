//
// Plugin Perspective Warp
//

#ifndef FLOWCV_PLUGIN_PERSPECTIVE_WARP_HPP_
#define FLOWCV_PLUGIN_PERSPECTIVE_WARP_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class PerspectiveWarp final : public Component
{
  public:
    PerspectiveWarp();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    int corner_count_;
    bool is_poly_;
    float ratio_adjustment_;
    int interp_mode_;
    bool use_fixed_res_;
    int fixed_width_;
    int fixed_height_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_PERSPECTIVE_WARP_HPP_
