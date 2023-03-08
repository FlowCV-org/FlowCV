//
// Plugin DNN Image Processing
//

#ifndef FLOWCV_PLUGIN_DNN_IMAGE_PROCESSING_HPP_
#define FLOWCV_PLUGIN_DNN_IMAGE_PROCESSING_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <ImGuiFileBrowser.h>
#include "dnn_backend_helper.hpp"

namespace DSPatch::DSPatchables
{
class ImageProcessing final : public Component
{
  public:
    ImageProcessing();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;
    void InitDnn_();

  private:
    DnnBackendListHelper dnn_backend_helper_;
    bool is_initialized_{};
    bool show_model_dialog_{};
    bool show_config_dialog_{};
    std::mutex io_mutex_;
    imgui_addons::ImGuiFileBrowser model_dialog_;
    imgui_addons::ImGuiFileBrowser config_dialog_;
    std::string model_path_;
    std::string config_path_;
    std::vector<std::pair<std::string, cv::dnn::Backend>> backend_list_;
    std::vector<std::pair<std::string, cv::dnn::Target>> target_list_;
    int dnn_backend_idx_;
    cv::dnn::Backend current_backend_;
    int dnn_target_idx_;
    cv::dnn::Target current_target_;
    std::unique_ptr<cv::dnn::Net> net_;
    int img_proc_mode_{};
    int img_proc_init_mode_{};
    float mean_[3]{};
    float scale_;
    int model_res_[2]{};
    int model_init_res_[2]{};
    bool net_load_error{};
    bool needs_reinit_{};
    bool swap_rb_{};
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_DNN_IMAGE_PROCESSING_HPP_
