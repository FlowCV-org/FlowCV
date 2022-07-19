//
// Plugin RndNoise
//

#ifndef FLOWCV_PLUGIN_RND_NOISE_HPP_
#define FLOWCV_PLUGIN_RND_NOISE_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class RndNoise final : public Component
{
  public:
    RndNoise();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    int width_;
    int height_;
    int mode_;
    std::mutex io_mutex_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_RND_NOISE_HPP_
