//
// Plugin Solid
//

#ifndef FLOWCV_PLUGIN_SOLID_HPP_
#define FLOWCV_PLUGIN_SOLID_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Solid final : public Component
{
  public:
    Solid();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    std::mutex io_mutex_;
    int width_{};
    int height_{};
    ImVec4 color_;
    int grey_value_;
    int fps_;
    int fps_index_;
    float fps_time_{};
    std::chrono::steady_clock::time_point last_time_;
    bool is_color_;
    bool has_alpha_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_SOLID_HPP_
