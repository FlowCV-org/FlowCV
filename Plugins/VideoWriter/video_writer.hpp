//
// Video Writer Plugin
//

#ifndef FLOWCV_VIDEO_WRITER_HPP_
#define FLOWCV_VIDEO_WRITER_HPP_

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
class VideoWriter;
}

class DLLEXPORT VideoWriter final : public Component
{
  public:
    VideoWriter();
    ~VideoWriter() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    void SaveSource();

  private:
    std::unique_ptr<internal::VideoWriter> p;
    cv::VideoWriter video_writer_;
    cv::Mat frame_;
    std::string out_filename_;
    std::string codec_str_;
    int codec_;
    int last_codec_;
    int fps_;
    int has_fps_in_;
    int last_fps_;
    int fps_index_;
    float fps_time_;
    std::chrono::steady_clock::time_point current_time_;
    std::chrono::steady_clock::time_point last_time_;
    imgui_addons::ImGuiFileBrowser file_dialog_;
    bool allow_write_;
    bool show_file_dialog_;
    bool save_new_file_;
    bool auto_ext_;

};

EXPORT_PLUGIN( VideoWriter )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_VIDEO_WRITER_HPP_
