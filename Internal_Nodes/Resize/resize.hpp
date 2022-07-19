//
// Plugin Resize
//

#ifndef FLOWCV_PLUGIN_RESIZE_HPP_
#define FLOWCV_PLUGIN_RESIZE_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Resize final : public Component
{
  public:
    Resize();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    bool has_size_input_;
    bool has_ref_frame_;
    int width_;
    int height_;
    int interp_mode_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_RESIZE_HPP_
