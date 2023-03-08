//
// Plugin DNN Classification
//

#ifndef FLOWCV_PLUGIN_DNN_CLASSIFICATION_HPP_
#define FLOWCV_PLUGIN_DNN_CLASSIFICATION_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <ImGuiFileBrowser.h>
#include "dnn_backend_helper.hpp"

namespace DSPatch::DSPatchables
{
class Classification final : public Component
{
  public:
    Classification();
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
    bool pre_proc_resize_{};
    bool show_model_dialog_{};
    bool show_config_dialog_{};
    bool show_classes_dialog_{};
    std::mutex io_mutex_;
    imgui_addons::ImGuiFileBrowser model_dialog_;
    imgui_addons::ImGuiFileBrowser config_dialog_;
    imgui_addons::ImGuiFileBrowser classes_dialog_;
    std::string config_path_;
    std::string model_path_;
    std::string classes_path_;
    std::vector<std::string> class_list_;
    std::vector<std::pair<std::string, cv::dnn::Backend>> backend_list_;
    std::vector<std::pair<std::string, cv::dnn::Target>> target_list_;
    int dnn_backend_idx_;
    cv::dnn::Backend current_backend_;
    int dnn_target_idx_;
    cv::dnn::Target current_target_;
    std::unique_ptr<cv::dnn::Net> net_;
    cv::Point2i text_pos_;
    float text_scale_;
    ImVec4 text_color_;
    int text_thickness_;
    float std_dev_[3]{};
    float mean_[3]{};
    float scale_;
    float conf_thresh_;
    int model_res_[2]{};
    bool net_load_error{};
    bool needs_reinit_{};
    bool swap_rb_{};
    bool needs_soft_max_{};
    bool draw_class_{};
    bool crop_{};
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_DNN_CLASSIFICATION_HPP_
