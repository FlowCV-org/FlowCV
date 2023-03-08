//
// Plugin Image Loader
//

#ifndef FLOWCV_IMAGE_LOADER_HPP_
#define FLOWCV_IMAGE_LOADER_HPP_
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
class ImageLoader;
}

class DLLEXPORT ImageLoader final : public Component
{
  public:
    ImageLoader();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    std::unique_ptr<internal::ImageLoader> p;
    cv::Mat frame_;
    bool show_file_dialog_;
    std::string image_file_;
    imgui_addons::ImGuiFileBrowser file_dialog_;
    int fps_;
    int fps_index_;
    float fps_time_{};
    std::chrono::steady_clock::time_point last_time_;
    std::mutex io_mutex_;
};

EXPORT_PLUGIN(ImageLoader)

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_IMAGE_LOADER_HPP_
