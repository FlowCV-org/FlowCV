//
// Simple Video Capture Plugin
//

#ifndef FLOWCV_VIDEO_CAPTURE_HPP_
#define FLOWCV_VIDEO_CAPTURE_HPP_

#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include "camera_enumerator.hpp"

namespace DSPatch::DSPatchables
{
namespace internal
{
class VideoCapture;
}

struct VideoMode
{
    int width;
    int height;
};

struct camera_property_info
{
    std::string name;
    float value{};
    bool supported{};
};

class DLLEXPORT VideoCapture final : public Component
{
  public:
    VideoCapture();
    ~VideoCapture() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    void OpenSource();
    void GetCameraProperties();
    void SetCameraProperties();

  private:
    std::unique_ptr<internal::VideoCapture> p;
    Camera_Enumerator cam_enum_;
    cv::VideoCapture cap_;
    cv::Mat frame_;
    std::vector<VideoMode> video_modes_;
    std::map<int, camera_property_info> cam_props_;
    nlohmann::json loaded_settings_;
    std::mutex io_mutex_;
    int fps_;
    int fps_index_;
    int src_mode_;
    int last_mode_;
    int last_fps_;
    int list_index_;
    int src_index_;
    int last_index_;
    bool restore_man_set_;
    bool first_load_set_;
    bool force_index_;
};

EXPORT_PLUGIN( VideoCapture )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_VIDEO_CAPTURE_HPP_
