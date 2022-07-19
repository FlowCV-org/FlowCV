//
// Plugin ShapeColorizer
//

#ifndef FLOWCV_PLUGIN_SHAPE_COLORIZER_HPP_
#define FLOWCV_PLUGIN_SHAPE_COLORIZER_HPP_
#define IMGUI_DEFINE_MATH_OPERATORS
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{
namespace internal
{
class ShapeColorizer;
}

struct ColorMatch
{
    ImVec4 col;
    char tag[32] = {'\0'};
    std::string ctl_label;
    std::string close_label;
    std::string text_label;
};

class DLLEXPORT ShapeColorizer final : public Component
{
  public:
    ShapeColorizer();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    std::unique_ptr<internal::ShapeColorizer> p;
    int method_;
    std::vector<ColorMatch> color_match_list_;
    int match_threshold_;
    int dist_method_;
    bool overlay_;
    bool show_labels_;
    bool ignore_none_;
    int ctl_cnt_;
    std::mutex io_mutex_;
};

EXPORT_PLUGIN( ShapeColorizer )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_SHAPE_COLORIZER_HPP_
