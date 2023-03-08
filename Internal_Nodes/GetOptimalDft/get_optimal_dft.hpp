//
// Plugin GetOptimalDft
//

#ifndef FLOWCV_PLUGIN_GET_OPTIMAL_DFT_HPP_
#define FLOWCV_PLUGIN_GET_OPTIMAL_DFT_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class GetOptimalDft final : public Component
{
  public:
    GetOptimalDft();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    bool has_size_input_;
    bool has_frame_input_;
    int width_;
    int height_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_GET_OPTIMAL_DFT_HPP_
