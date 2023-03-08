//
// Plugin BlobDetector
//

#ifndef FLOWCV_PLUGIN_BLOB_DETECTOR_HPP_
#define FLOWCV_PLUGIN_BLOB_DETECTOR_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class BlobDetector final : public Component
{
  public:
    BlobDetector();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    ImVec4 blob_viz_color_;
    int blob_param_color_;
    int blob_param_repeat_;
    cv::SimpleBlobDetector::Params blob_params_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_BLOB_DETECTOR_HPP_
