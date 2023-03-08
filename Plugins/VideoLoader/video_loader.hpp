//
// Plugin MediaReader
//

#ifndef FLOWCV_PLUGIN_MEDIA_READER_HPP_
#define FLOWCV_PLUGIN_MEDIA_READER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <ImGuiFileBrowser.h>

namespace DSPatch::DSPatchables
{
namespace internal
{
class VideoLoader;
}

class DLLEXPORT VideoLoader final : public Component
{
  public:
    VideoLoader();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    void OpenSource();
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    std::unique_ptr<internal::VideoLoader> p;
    cv::VideoCapture cap_;
    bool show_file_dialog_;
    int play_mode_;
    bool loop_;
    std::string video_file_;
    int frame_count_;
    bool start_;
    std::mutex io_mutex_;
    int cur_frame_;
    int last_frame_{};
    bool load_new_file_;
    bool use_fps_;
    int fps_{};
    float fps_time_{};
    std::chrono::steady_clock::time_point current_time_;
    std::chrono::steady_clock::time_point last_time_;
    imgui_addons::ImGuiFileBrowser file_dialog_;
};

EXPORT_PLUGIN(VideoLoader)

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_MEDIA_READER_HPP_
