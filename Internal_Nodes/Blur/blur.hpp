//
// Various OpenCV Blur Filters
//

#ifndef FLOWCV_BLUR_HPP_
#define FLOWCV_BLUR_HPP_
#include <DSPatch.h>
#include <FlowCV_Types.hpp>
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Blur final : public Component
{
  public:
    Blur();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    float blur_amt_h_;
    float blur_amt_v_;
    bool lock_h_v_;
    int blur_mode_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_GAUSSIAN_BLUR_HPP_
