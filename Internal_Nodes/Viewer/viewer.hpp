//
// Created by Richard on 1/3/2022.
//

#ifndef FLOWCV_VIEWER_HPP_
#define FLOWCV_VIEWER_HPP_
#include <mutex>
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"

namespace DSPatch::DSPatchables
{

class Viewer final : public Component
{
  public:
    Viewer();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    cv::Mat frame_;
    bool has_update_;
    ImGuiOpenCvWindow viewer_;
    std::mutex io_mutex_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_VIEWER_HPP_
