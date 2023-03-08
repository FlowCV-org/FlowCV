//
// Plugin HoughLines
//

#ifndef FLOWCV_PLUGIN_HOUGH_LINES_HPP_
#define FLOWCV_PLUGIN_HOUGH_LINES_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class HoughLines final : public Component
{
  public:
    HoughLines();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int hough_mode_;
    float rho_;
    int thresh_;
    float srn_;
    float stn_;
    float min_theta_;
    float max_theta_;
    float min_line_len_;
    float max_line_gap_;
    ImVec4 line_color_;
    int line_thickness_;
    bool has_input_;
    bool valid_format_;
    bool draw_lines_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_HOUGH_LINES_HPP_
