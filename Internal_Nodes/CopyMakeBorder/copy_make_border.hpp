//
// Plugin CopyMakeBorder
//

#ifndef FLOWCV_PLUGIN_COPY_MAKE_BORDER_HPP_
#define FLOWCV_PLUGIN_COPY_MAKE_BORDER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class CopyMakeBorder final : public Component
{
  public:
    CopyMakeBorder();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int add_width_;
    int add_height_;
    int border_type_;
    ImVec4 border_color_;
    bool center_;
    bool has_size_input_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_COPY_MAKE_BORDER_HPP_
