//
// Plugin DrawNumber
//

#ifndef FLOWCV_PLUGIN_DRAW_NUMBER_HPP_
#define FLOWCV_PLUGIN_DRAW_NUMBER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class DrawNumber final : public Component
{
 public:
    DrawNumber();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

 protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

 private:
    cv::Point2i text_pos_;
    float text_scale_;
    ImVec4 text_color_;
    int text_thickness_;
    int text_font_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_DRAW_NUMBER_HPP_
