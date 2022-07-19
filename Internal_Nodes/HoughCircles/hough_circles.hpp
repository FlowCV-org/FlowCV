//
// Plugin HoughCircles
//

#ifndef FLOWCV_PLUGIN_HOUGH_CIRCLES_HPP_
#define FLOWCV_PLUGIN_HOUGH_CIRCLES_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class HCircles final : public Component
{
  public:
    HCircles();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    float dp_;
    float min_dist_;
    float param1_;
    float param2_;
    int min_radius_;
    int max_radius_;
    ImVec4 circle_color_;
    ImVec4 center_color_;
    ImVec4 text_color_;
    float text_scale_;
    int text_thickness_;
    int text_offset_;
    bool fill_circle_;
    bool show_radius_;
    bool show_center_;
    int circle_thickness_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_HOUGH_CIRCLES_HPP_
