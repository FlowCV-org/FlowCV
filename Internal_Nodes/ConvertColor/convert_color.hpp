//
// Plugin ConvertColor
//

#ifndef FLOWCV_PLUGIN_CONVERT_COLOR_HPP_
#define FLOWCV_PLUGIN_CONVERT_COLOR_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class ConvertColor final : public Component
{
  public:
    ConvertColor();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int cvt_mode_;
    bool channel_error_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_CONVERT_COLOR_HPP_
