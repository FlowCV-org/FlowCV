//
// Plugin LaplacianFilter
//

#ifndef FLOWCV_PLUGIN_LAPLACIAN_FILTER_HPP_
#define FLOWCV_PLUGIN_LAPLACIAN_FILTER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"

namespace DSPatch::DSPatchables
{

class LaplacianFilter final : public Component
{
  public:
    LaplacianFilter();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;

  private:
    int out_depth_;
    int ksize_;
    float scale_;
    float delta_;
};

}  // namespace DSPatch::DSPatchables

#endif  // FLOWCV_PLUGIN_LAPLACIAN_FILTER_HPP_
