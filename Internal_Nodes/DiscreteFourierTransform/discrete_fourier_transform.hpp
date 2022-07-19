//
// Plugin DiscreteFourierTransform
//

#ifndef FLOWCV_PLUGIN_DISCRETE_FOURIER_TRANSFORM_HPP_
#define FLOWCV_PLUGIN_DISCRETE_FOURIER_TRANSFORM_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class DiscreteFourierTransform final : public Component
{
  public:
    DiscreteFourierTransform();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    int mode_;
    bool log_view_;
    bool shift_dc_center_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_DISCRETE_FOURIER_TRANSFORM_HPP_
