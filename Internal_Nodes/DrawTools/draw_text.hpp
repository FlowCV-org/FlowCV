//
// Plugin DrawText
//

#ifndef FLOWCV_PLUGIN_DRAW_TEXT_HPP_
#define FLOWCV_PLUGIN_DRAW_TEXT_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class DrawText final : public Component
{
  public:
    DrawText();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    char text_[128];
    cv::Point2i text_pos_;
    float text_scale_;
    ImVec4 text_color_;
    int text_thickness_;
    bool has_str_input_;
    int text_font_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_DRAW_TEXT_HPP_
