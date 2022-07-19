//
// Plugin Crop
//

#ifndef FLOWCV_PLUGIN_CROP_HPP_
#define FLOWCV_PLUGIN_CROP_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Crop final : public Component
{
  public:
    Crop();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    cv::Rect crop_area_;
    bool has_json_data_;
    bool calc_total_bbox_;
    int json_bbox_index_;
    int adjust_size_x_;
    int adjust_size_y_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_CROP_HPP_
