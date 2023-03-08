//
// Plugin ImageWriter
//

#ifndef FLOWCV_PLUGIN_IMAGE_WRITER_HPP_
#define FLOWCV_PLUGIN_IMAGE_WRITER_HPP_
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
class ImageWriter;
}

class DLLEXPORT ImageWriter final : public Component
{
  public:
    ImageWriter();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    std::unique_ptr<internal::ImageWriter> p;
    cv::Mat frame_;
    bool show_file_dialog_;
    bool save_image_now_;
    std::string image_file_;
    imgui_addons::ImGuiFileBrowser file_dialog_;
};

EXPORT_PLUGIN(ImageWriter)

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_IMAGE_WRITER_HPP_
