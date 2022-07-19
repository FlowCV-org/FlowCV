//
// Plugin DepthViewer3D
//

#ifndef FLOWCV_PLUGIN_DEPTH_VIEWER_3D_HPP_
#define FLOWCV_PLUGIN_DEPTH_VIEWER_3D_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "imgui_3d_opengl.hpp"
#include "json.hpp"

struct colorVertNorm {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    GLfloat nx;
    GLfloat ny;
    GLfloat nz;
};

namespace DSPatch::DSPatchables
{

class DepthViewer3D final : public Component
{
  public:
    DepthViewer3D();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    void InitOpenGLWin_();
    void InitPointCloud_();

  private:
    cv::Mat depth_frame_;
    cv::Mat color_frame_;
    nlohmann::json intrinsic_data_;
    std::shared_ptr<ImGuiOpenGlObjectBuffer> pntCloudObj;
    std::vector<colorVertNorm> pntCloudVerts;
    bool ogl_init_;
    bool pnt_init_;
    bool diff_shading_;
    bool has_intrinsic_;
    float no_intrinsic_hfov_;
    glm::vec3 pos_offset_;
    ImVec4 diff_color_;
    std::mutex io_mutex_;
    std::shared_ptr<ImGuiOpenGlWindow> ogl_win_;
};

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_DEPTH_VIEWER_3D_HPP_
