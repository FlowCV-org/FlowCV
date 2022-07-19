//
// Plugin HistogramViewer
//

#ifndef FLOWCV_PLUGIN_HISTOGRAM_VIEWER_HPP_
#define FLOWCV_PLUGIN_HISTOGRAM_VIEWER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class HistogramViewer final : public Component
{
  public:
    HistogramViewer();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    cv::Mat frame_;
    bool has_input_;
    bool is_color_;
    std::mutex io_mutex_;
    std::vector<float> x_range_;
    std::vector<float> values_r_;
    std::vector<float> values_g_;
    std::vector<float> values_b_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_HISTOGRAM_VIEWER_HPP_
