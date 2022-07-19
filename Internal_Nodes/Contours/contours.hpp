//
// Plugin Contours
//

#ifndef FLOWCV_PLUGIN_CONTOURS_HPP_
#define FLOWCV_PLUGIN_CONTOURS_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Contours final : public Component
{
  public:
    Contours();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    int mode_;
    int method_;
    bool filter_by_area_;
    float area_min_;
    float area_max_;
    bool draw_contours_;
    bool fill_contours_;
    bool draw_bbox_;
    ImVec4 contour_color_;
    ImVec4 bbox_color_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_CONTOURS_HPP_
