//
// Plugin BackgroundSubtraction
//

#ifndef FLOWCV_PLUGIN_BACKGROUND_SUBTRACTION_HPP_
#define FLOWCV_PLUGIN_BACKGROUND_SUBTRACTION_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class BackgroundSubtraction final : public Component
{
  public:
    BackgroundSubtraction();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> bg_subtractor_mog2_;
    cv::Ptr<cv::BackgroundSubtractorKNN> bg_subtractor_knn_;
    bool update_settings_;
    int bkg_sub_mode_;
    bool detect_shadows_;
    float threshold_;
    int history_;

};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_BACKGROUND_SUBTRACTION_HPP_
