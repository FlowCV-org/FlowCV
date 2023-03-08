//
// Plugin Morphology
//

#ifndef FLOWCV_PLUGIN_MORPHOLOGY_HPP_
#define FLOWCV_PLUGIN_MORPHOLOGY_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class Morphology final : public Component
{
  public:
    Morphology();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int op_;
    int iterations_;
    int border_type_;
    int morph_amt_;
    int morph_shape_;
    ImVec4 border_color_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_MORPHOLOGY_HPP_
