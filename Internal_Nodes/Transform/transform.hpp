//
// Plugin Transform
//

#ifndef FLOWCV_PLUGIN_TRANSFORM_HPP_
#define FLOWCV_PLUGIN_TRANSFORM_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Transform final : public Component
{
  public:
    Transform();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    cv::Point2i trans_;
    cv::Point2i frame_res_;
    float aspect_ratio_;
    int rotate_mode_;
    float rotate_amt_;
    int interp_mode_;
    int aspect_mode_;
    int flip_mode_;
    cv::Point2f scale_;
    cv::Point2f scale_max_;
    int scale_mode_;
    std::mutex io_mutex_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_TRANSFORM_HPP_
