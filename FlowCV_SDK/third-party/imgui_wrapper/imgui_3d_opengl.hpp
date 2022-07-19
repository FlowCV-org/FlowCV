//
// ImGUI 3D OpenGL Display Class
// Created by Richard Wardlow
//

#ifndef IMGUI_WRAPPER_IMGUI_3D_OPENGL_HPP_
#define IMGUI_WRAPPER_IMGUI_3D_OPENGL_HPP_
#include "imgui_wrapper.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// Vertex Shader source code
extern const char* ImGuiOpenGlVertexShaderSource;

//Fragment Shader source code
extern const char* ImGuiOpenGlFragmentShaderSource;

struct ImGuiOpenGlColorVert {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

struct ImGuiOpenGlObjectBuffer {
    GLuint shader{};
    GLenum obj_type{};
    GLsizei elem_size{};
    GLfloat draw_point_size{};
    std::string obj_name;
    std::shared_ptr<VAO> vao;
    std::shared_ptr<VBO> vbo;
    std::shared_ptr<EBO> ebo;
};

class ImGuiOpenGlWindow
{
  public:
    explicit ImGuiOpenGlWindow(int width = 1280, int height = 720 );
    ~ImGuiOpenGlWindow();
    bool Init();
    void Update(const char *title);
    void AddObject(std::shared_ptr<ImGuiOpenGlObjectBuffer> object);
    void DeleteObject(const std::string &object_name);
    [[nodiscard]] const ImVec4 GetViewBgColor() const;
    void SetViewBgColor(const ImVec4 &view_bg_color);
    [[nodiscard]] float GetGridSize() const;
    void SetGridSize(float grid_size);
    [[nodiscard]] bool IsShowGrid() const;
    void SetShowGrid(bool show_grid);
    [[nodiscard]] const glm::vec3 GetViewPos() const;
    void SetViewPos(const glm::vec3 &view_pos);
    [[nodiscard]] const glm::vec3 GetViewCenter() const;
    void SetViewCenter(const glm::vec3 &view_center);
    [[nodiscard]] float GetCamMoveSpeed() const;
    void SetCamMoveSpeed(float cam_move_speed);
    [[nodiscard]] float GetCamFov() const;
    void SetCamFov(float cam_fov);
    [[nodiscard]] float GetYaw() const;
    void SetYaw(float yaw);
    [[nodiscard]] float GetPitch() const;
    void SetPitch(float pitch);
    [[nodiscard]] float GetLat() const;
    void SetLat(float lat);
    [[nodiscard]] float GetLon() const;
    void SetLon(float lon);
    [[nodiscard]] float GetZoom() const;
    void SetZoom(float zoom);
    [[nodiscard]] bool IsOrbitMode() const;
    void SetOrbitMode(bool orbit_mode);
    [[nodiscard]] const glm::vec3 GetCamFwd() const;
    void SetCamFwd(const glm::vec3 &cam_fwd);
    [[nodiscard]] const glm::vec3 GetCamUp() const;
    void SetCamUp(const glm::vec3 &cam_up);
    [[nodiscard]] bool IsUseMsaa() const;
    void SetUseMsaa(bool use_msaa);

 protected:
    void GenerateGrid_();
    void CalculateOrbitPos_();
    void CalculateFlyPos_();
    void FlyToOrbit_();
    void OrbitToFly_();
    static bool IsMouseInWindow_(ImVec2 min, ImVec2 max);

  private:
    const float pi_ = 3.1415926535897932384626433832795f;
    std::shared_ptr<VBO> grid_vbo_;
    std::shared_ptr<VAO> grid_vao_;

 private:
    std::shared_ptr<EBO> grid_ebo_;
    std::vector<ImGuiOpenGlColorVert> grid_verts_;
    std::vector<glm::uvec4> grid_indices_;
    std::vector<std::shared_ptr<ImGuiOpenGlObjectBuffer>> draw_items_;
    GLuint grid_shader_{};
    GLuint frame_buffer_{};
    GLuint frame_buffer_texture_{};
    GLuint frame_buffer_post_{};
    GLuint frame_buffer_texture_post_{};
    GLuint render_buffer_object_{};
    GLuint render_buffer_object_no_msaa_{};
    ImVec2 texture_size_;
    ImVec2 view_size_;
    ImVec4 view_bg_color_;
    float view_ratio_;
    float grid_size_;
    int grid_slices_;
    bool show_grid_;
    bool is_init_;
    bool use_msaa_;
    glm::vec3 view_pos_{};
    glm::vec3 view_center_{};
    glm::vec3 cam_fwd_{};
    glm::vec3 cam_up_{};
    float cam_move_speed_;
    float cam_fov_;
    float orb_rad_;
    float yaw_;
    float pitch_;
    float lat_;
    float lon_;
    float zoom_;
    bool orbit_mode_;


};

#endif //IMGUI_WRAPPER_IMGUI_3D_OPENGL_HPP_
