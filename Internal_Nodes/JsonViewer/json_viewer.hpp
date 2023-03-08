//
// Plugin JsonViewer
//

#ifndef FLOWCV_PLUGIN_JSON_VIEWER_HPP_
#define FLOWCV_PLUGIN_JSON_VIEWER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class JsonViewer final : public Component
{
  public:
    JsonViewer();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    nlohmann::json json_data_;
    std::mutex io_mutex_;
    bool show_raw_out_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_JSON_VIEWER_HPP_
