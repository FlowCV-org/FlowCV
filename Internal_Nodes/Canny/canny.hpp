//
// Plugin Canny
//

#ifndef FLOWCV_PLUGIN_CANNY_HPP_
#define FLOWCV_PLUGIN_CANNY_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "FlowCV_Properties.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class CannyFilter final : public Component
{
  public:
    CannyFilter();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    FlowCV::FlowCV_Properties props_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_CANNY_HPP_
