//
// Plugin DNN Segmentation
//

#ifndef FLOWCV_PLUGIN_DNN_SEGMENTATION_HPP_
#define FLOWCV_PLUGIN_DNN_SEGMENTATION_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <ImGuiFileBrowser.h>
#include "dnn_backend_helper.hpp"

namespace DSPatch::DSPatchables
{
class Segmentation final : public Component
{
  public:
    Segmentation();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    void InitDnn_();
    void ColorizeSegmentation_(const cv::Mat &score, cv::Mat &segm);

  private:
    DnnBackendListHelper dnn_backend_helper_;
    bool is_initialized_{};
    bool pre_proc_resize_{};
    bool show_model_dialog_{};
    bool show_config_dialog_{};
    bool show_classes_dialog_{};
    bool show_colors_dialog_{};
    std::mutex io_mutex_;
    imgui_addons::ImGuiFileBrowser model_dialog_;
    imgui_addons::ImGuiFileBrowser config_dialog_;
    imgui_addons::ImGuiFileBrowser classes_dialog_;
    imgui_addons::ImGuiFileBrowser colors_dialog_;
    std::string config_path_;
    std::string model_path_;
    std::string classes_path_;
    std::string colors_path_;
    std::vector<std::string> class_list_;
    std::vector<cv::Vec3b> colors_list_;
    std::vector<std::pair<std::string, cv::dnn::Backend>> backend_list_;
    std::vector<std::pair<std::string, cv::dnn::Target>> target_list_;
    std::vector<cv::String> out_names_;
    int dnn_backend_idx_;
    cv::dnn::Backend current_backend_;
    int dnn_target_idx_;
    cv::dnn::Target current_target_;
    std::unique_ptr<cv::dnn::Net> net_;
    float mean_[3]{};
    float scale_;
    int model_res_[2]{};
    bool net_load_error{};
    bool needs_reinit_{};
    bool swap_rb_{};
};

}  // namespace DSPatch::DSPatchables

#endif // FLOWCV_PLUGIN_DNN_SEGMENTATION_HPP_
