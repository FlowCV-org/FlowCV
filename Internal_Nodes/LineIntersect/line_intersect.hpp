//
// Plugin Line Intersections
//

#ifndef FLOWCV_PLUGIN_LINE_INTERSECT_HPP_
#define FLOWCV_PLUGIN_LINE_INTERSECT_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class LineIntersect final : public Component
{
  public:
    LineIntersect();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    float min_angle_;
    float min_dist_;
    ImVec4 point_color_;
    int point_radius_;
    int point_thickness_;
    int intersect_count_;
    int line_count_;
    bool draw_intersections_;
    bool point_solid_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_LINE_INTERSECT_HPP_
