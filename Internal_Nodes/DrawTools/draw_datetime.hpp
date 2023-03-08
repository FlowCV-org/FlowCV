//
// Plugin DrawDateTime
//

#ifndef FLOWCV_PLUGIN_DRAW_DATETIME_HPP_
#define FLOWCV_PLUGIN_DRAW_DATETIME_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class DrawDateTime final : public Component
{
  public:
    DrawDateTime();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;
    bool IsFormatValid(int mode);

  private:
    cv::Point2i text_pos_;
    float text_scale_;
    ImVec4 text_color_;
    int text_thickness_;
    int text_font_;
    bool draw_date_;
    bool draw_time_;
    char date_format_[32];
    char time_format_[32];
    bool is_valid_date_format_;
    bool is_valid_time_format_;
    std::string date_format_str_;
    std::string time_format_str_;
    std::mutex io_mutex_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_DRAW_DATETIME_HPP_
