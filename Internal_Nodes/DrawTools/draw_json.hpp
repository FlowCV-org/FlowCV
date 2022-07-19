//
// Plugin DrawJSON
//

#ifndef FLOWCV_PLUGIN_DRAW_JSON_HPP_
#define FLOWCV_PLUGIN_DRAW_JSON_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

struct JsonOutItem
{
    int out_item_index{};
    std::string out_key_path;
    bool is_array{};
};

class DrawJson final : public Component
{
 public:
    DrawJson();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

 protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    std::string JsonTreeToStringList_(const nlohmann::json::iterator &it, std::string &curDepth);
    bool IsInList_(const std::string &key_path);

 private:
    nlohmann::json json_data_;
    cv::Point2i text_pos_;
    int json_data_index_;
    std::vector<std::string> json_tree_list_;
    std::vector<JsonOutItem> json_out_list_;
    float text_scale_;
    int float_draw_precision_;
    ImVec4 text_color_;
    int text_thickness_;
    int text_font_;
    std::mutex io_mutex_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_DRAW_JSON_HPP_
