//
// Plugin SimpleBlobTracker
//

#ifndef FLOWCV_PLUGIN_SIMPLE_BLOB_TRACKER_HPP_
#define FLOWCV_PLUGIN_SIMPLE_BLOB_TRACKER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{
namespace internal
{

struct BlobTrackingData
{
    std::deque<cv::Point> pos;
    cv::Vec3b color;
    uint32_t id;
    int last_size;
    uint32_t last_frame;

};

class SimpleBlobTracker;
}

class DLLEXPORT SimpleBlobTracker final : public Component
{
  public:
    SimpleBlobTracker();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    std::unique_ptr<internal::SimpleBlobTracker> p;
    std::vector<internal::BlobTrackingData> blob_tracking_;
    bool reset_tracking_;
    bool show_tracking_;
    bool show_ids_;
    uint32_t id_counter_;
    uint32_t frame_count_;
    int tracking_duration_;
    bool drop_lost_tracks_;
    int lost_track_duration_;
    int max_pos_var_;
    int max_size_var_;
};

EXPORT_PLUGIN( SimpleBlobTracker )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_SIMPLE_BLOB_TRACKER_HPP_
