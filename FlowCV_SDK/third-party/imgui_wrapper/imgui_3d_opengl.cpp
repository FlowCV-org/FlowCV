//
// ImGUI 3D OpenGL Display Class
// Created by Richard Wardlow
//

#include "imgui_3d_opengl.hpp"
# include <imgui_internal.h>

// Vertex Shader source code
const char* ImGuiOpenGlVertexShaderSource = "#version 330 core\n"
                                            "layout (location = 0) in vec3 aPos;\n"
                                            "layout (location = 1) in vec4 aColor;\n"
                                            "out vec4 color;\n"
                                            "uniform float size;\n"
                                            "uniform mat4 model;\n"
                                            "uniform mat4 view;\n"
                                            "uniform mat4 proj;\n"
                                            "void main()\n"
                                            "{\n"
                                            "   gl_Position = proj * view * model * vec4(size * aPos.x, size * aPos.y, size * aPos.z, 1.0);\n"
                                            "   color = aColor;\n"
                                            "}\0";
//Fragment Shader source code
const char* ImGuiOpenGlFragmentShaderSource = "#version 330 core\n"
                                              "out vec4 FragColor;\n"
                                              "in vec4 color;\n"
                                              "void main()\n"
                                              "{\n"
                                              "   FragColor = color;\n"
                                              "}\n\0";

ImGuiOpenGlWindow::ImGuiOpenGlWindow(int width, int height)
{
    texture_size_.x = (float)width;
    texture_size_.y = (float)height;
    view_pos_ = {0.0f, 0.0f, 0.0f};
    view_center_ = {0.0f, 3.7f, 0.0f};
    cam_fwd_ = {0.00483705, 0.118993, 0.992883};
    cam_up_ = {0.0f, 1.0f,  0.0f};
    yaw_ = 88.5f;
    pitch_ = 8.88f;
    lat_ = 0.5437f;
    lon_ = -0.2505f;
    zoom_ = 13.5f;
    grid_size_ = 1.0f;
    show_grid_ = true;
    orbit_mode_ = true;
    use_msaa_ = false;
    cam_move_speed_ = 0.1f;
    cam_fov_ = 45.0f;
    grid_slices_ = 10;
    view_size_.x = texture_size_.x;
    view_size_.y = texture_size_.y;
    view_ratio_ = view_size_.x / view_size_.y;
    view_bg_color_.x = 22/255.0f;
    view_bg_color_.y = 26/255.0f;
    view_bg_color_.z = 33/255.0f;
    view_bg_color_.w = 1.0f;
    is_init_ = false;

    orb_rad_ = (cos((lat_ - 0.5f) * pi_));
    view_pos_.x = (cos(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;
    view_pos_.y = (sin((lat_ - 0.5f) * pi_)) * zoom_;
    view_pos_.z = (sin(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;

}

ImGuiOpenGlWindow::~ImGuiOpenGlWindow()
{
    if (is_init_) {
        glDeleteVertexArrays(1, &grid_vao_->ID);
        glDeleteBuffers(1, &grid_vbo_->ID);
        glDeleteBuffers(1, &grid_ebo_->ID);
        glDeleteTextures(1, &frame_buffer_texture_);
        glDeleteShader(grid_shader_);
    }
}

bool ImGuiOpenGlWindow::Init()
{
    bool res = false;

    glGenFramebuffers(1, &frame_buffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);

    // Create Framebuffer Texture
    glGenTextures(1, &frame_buffer_texture_);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frame_buffer_texture_);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, (GLsizei)texture_size_.x, (GLsizei)texture_size_.y, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frame_buffer_texture_, 0);

    // Create Render Buffer Object
    glGenRenderbuffers(1, &render_buffer_object_);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT24, (GLsizei)texture_size_.x, (GLsizei)texture_size_.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object_);

    glGenFramebuffers(1, &frame_buffer_post_);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_post_);

    // Create Framebuffer Texture
    glGenTextures(1, &frame_buffer_texture_post_);
    glBindTexture(GL_TEXTURE_2D, frame_buffer_texture_post_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texture_size_.x, (GLsizei)texture_size_.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_buffer_texture_post_, 0);

    // Create Render Buffer Object
    glGenRenderbuffers(1, &render_buffer_object_no_msaa_);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object_no_msaa_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (GLsizei)texture_size_.x, (GLsizei)texture_size_.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object_no_msaa_);

    // Error checking framebuffer
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus == GL_FRAMEBUFFER_COMPLETE) {
        res = true;
        is_init_ = res;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (res)
        GenerateGrid_();

    return res;
}

void ImGuiOpenGlWindow::GenerateGrid_()
{
    grid_verts_.clear();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &ImGuiOpenGlVertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &ImGuiOpenGlFragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    grid_shader_ = glCreateProgram();
    glAttachShader(grid_shader_, vertexShader);
    glAttachShader(grid_shader_, fragmentShader);
    glLinkProgram(grid_shader_);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    for(int j=0; j<=grid_slices_; ++j) {
        for(int i=0; i<=grid_slices_; ++i) {
            float x = (((float)i/(float)grid_slices_) - 0.5f) * 10.0f;
            float y = 0;
            float z = (((float)j/(float)grid_slices_) - 0.5f) * 10.0f;
            ImGuiOpenGlColorVert vert{};
            vert.x = x;
            vert.y = y;
            vert.z = z;

            // color
            if (i == (int)(grid_slices_ / 2) || j == (int)(grid_slices_ / 2)){
                vert.r = 0.55f;
                vert.g = 0.55f;
                vert.b = 0.55f;
                vert.a = 1.0f;
            }
            else {
                vert.r = 0.25f;
                vert.g = 0.25f;
                vert.b = 0.25f;
                vert.a = 0.75f;
            }
            grid_verts_.emplace_back(vert);
        }
    }

    for(int j=0; j<grid_slices_; ++j) {
        for(int i=0; i<grid_slices_; ++i) {

            int row1 =  j    * (grid_slices_+1);
            int row2 = (j+1) * (grid_slices_+1);

            grid_indices_.emplace_back(glm::uvec4(row1+i, row1+i+1, row1+i+1, row2+i+1));
            grid_indices_.emplace_back(glm::uvec4(row2+i+1, row2+i, row2+i, row1+i));

        }
    }

    grid_vao_ = std::make_shared<VAO>();
    grid_vbo_ = std::make_shared<VBO>();
    grid_ebo_ = std::make_shared<EBO>();

    glUseProgram(grid_shader_);
    grid_vao_->Set();
    grid_vao_->Bind();
    grid_vbo_->Set((GLfloat *)grid_verts_.data(), (GLsizei)(grid_verts_.size() * sizeof(ImGuiOpenGlColorVert)));
    grid_ebo_->Set(glm::value_ptr(grid_indices_[0]), (GLsizei)(grid_indices_.size()*sizeof(glm::uvec4)));
    grid_vao_->LinkAttrib(grid_vbo_, 0, 3, GL_FLOAT, 7 * sizeof(GLfloat), nullptr);
    grid_vao_->LinkAttrib(grid_vbo_, 1, 4, GL_FLOAT, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    grid_vao_->Unbind();
    grid_vbo_->Unbind();
    grid_ebo_->Unbind();
    glUseProgram(0);
}

void ImGuiOpenGlWindow::Update(const char *title)
{
    // Bind the custom framebuffer
    if (use_msaa_)
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_post_);
    // Specify the color of the background
    glClearColor(view_bg_color_.x, view_bg_color_.y, view_bg_color_.z, view_bg_color_.w);
    // Clean the back buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // Initializes matrices so they are not the null matrix
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 look;
    if (orbit_mode_)
        look = glm::lookAt(view_pos_, view_center_, glm::vec3(0.0f, 1.0f, 0.0f));
    else {
        view = glm::lookAt(view_pos_, view_pos_ + cam_fwd_, cam_up_);
    }
    proj = glm::perspective(glm::radians(cam_fov_), view_ratio_, 0.1f, 5000.0f);

    glUseProgram(grid_shader_);
    glUniform1f(glGetUniformLocation(grid_shader_, "size"), grid_size_);
    int gridLoc = glGetUniformLocation(grid_shader_, "model");
    glUniformMatrix4fv(gridLoc, 1, GL_FALSE, glm::value_ptr(model));
    int viewLoc = glGetUniformLocation(grid_shader_, "view");
    if (orbit_mode_)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(look));
    else
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    int projLoc = glGetUniformLocation(grid_shader_, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    grid_vao_->Bind();
    if (show_grid_)
        glDrawElements(GL_LINES, (GLsizei)grid_indices_.size()*4, GL_UNSIGNED_INT, nullptr);

    grid_vao_->Unbind();
    glUseProgram(0);

    for (auto &obj : draw_items_) {
        glUseProgram(obj->shader);
        glUniform1f(glGetUniformLocation(obj->shader, "size"), 1.0f);
        int tmpModelLoc = glGetUniformLocation(obj->shader, "model");
        glUniformMatrix4fv(tmpModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        int tmpViewLoc = glGetUniformLocation(obj->shader, "view");
        if (orbit_mode_)
            glUniformMatrix4fv(tmpViewLoc, 1, GL_FALSE, glm::value_ptr(look));
        else
            glUniformMatrix4fv(tmpViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int tmpProjLoc = glGetUniformLocation(obj->shader, "proj");
        glUniformMatrix4fv(tmpProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        obj->vao->Bind();
        if (obj->obj_type == GL_TRIANGLES)
            glDrawElements(GL_TRIANGLES, obj->elem_size, GL_UNSIGNED_INT, nullptr);
        else if (obj->obj_type == GL_LINES)
            glDrawElements(GL_LINES, obj->elem_size, GL_UNSIGNED_INT, nullptr);
        else if (obj->obj_type == GL_POINTS) {
            glPointSize(obj->draw_point_size);
            glDrawArrays(GL_POINTS, 0, obj->elem_size);
        }
        obj->vao->Unbind();
        glUseProgram(0);
    }

    if (use_msaa_) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_post_);
        // Conclude the multisampling and copy it to the post-processing FBO
        glBlitFramebuffer(0, 0, (GLsizei) texture_size_.x, (GLsizei) texture_size_.y, 0, 0, (GLsizei) texture_size_.x, (GLsizei) texture_size_.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    ImGuiIO &io = ImGui::GetIO();

    ImGui::SetNextWindowSizeConstraints(ImVec2(720.f, 200.f), ImVec2(INFINITY, INFINITY));
    ImGui::Begin(title);
    view_size_ = ImGui::GetWindowSize();
    view_ratio_ = view_size_.x / view_size_.y;

    // Controls
    ImGui::Text("Camera: ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    ImGui::DragFloat("FOV", &cam_fov_, 0.5f, 1.0f, 100.0f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    if (ImGui::Checkbox("Orbit", &orbit_mode_)) {
        if (orbit_mode_) {
            FlyToOrbit_();
        }
        else {
            OrbitToFly_();
        }
    }
    ImGui::SameLine();
    if (!orbit_mode_) {
        ImGui::SetNextItemWidth(60);
        ImGui::DragFloat("Fly Speed", &cam_move_speed_, 0.01f, 0.01f, 10.0f);
        ImGui::SameLine();
    }
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::Text("Grid: ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::Checkbox("Show", &show_grid_);
    if (show_grid_) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragFloat("Size", &grid_size_, 0.05f, 0.1f, 100.0f);
    }
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::Checkbox("MSAA", &use_msaa_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    ImGui::ColorEdit3("BG Color", (float*)&view_bg_color_, ImGuiColorEditFlags_NoInputs);

    // Display Frame Buffer Render
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 imgSize = ImVec2(pos.x + (view_size_.x - 18), pos.y + (view_size_.y - 60));
    ImGui::GetWindowDrawList()->AddImage(
        (void*)(intptr_t)frame_buffer_texture_post_, pos, imgSize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    if (IsMouseInWindow_(pos, imgSize)) {
        if (io.MouseDown[1]) {
            if (orbit_mode_) {
                lat_ += (io.MouseDelta.y * 0.002f);
                lon_ += (io.MouseDelta.x * 0.001f);
                if (lat_ < 0.02f)
                    lat_ = 0.02f;
                if (lat_ > 0.98f)
                    lat_ = 0.98f;
                CalculateOrbitPos_();
            } else {
                if (io.KeyCtrl) {
                    view_pos_ += cam_up_ * (io.MouseDelta.y * 0.1f) * cam_move_speed_;
                    view_pos_ -= glm::normalize(glm::cross(cam_fwd_, cam_up_)) * (io.MouseDelta.x * 0.1f) * cam_move_speed_;
                } else {
                    pitch_ -= (io.MouseDelta.y * 0.1f);
                    yaw_ += (io.MouseDelta.x * 0.1f);
                    if (pitch_ < -89.99f)
                        pitch_ = -89.99f;
                    else if (pitch_ > 89.99f)
                        pitch_ = 89.99f;
                    CalculateFlyPos_();
                }
            }
        } else if (io.MouseDown[2]) {
            if (orbit_mode_)
                view_center_.y += (io.MouseDelta.y * 0.01f);
            else {
                view_pos_ += cam_up_ * (io.MouseDelta.y * 0.1f) * cam_move_speed_;
                view_pos_ -= glm::normalize(glm::cross(cam_fwd_, cam_up_)) * (io.MouseDelta.x * 0.1f) * cam_move_speed_;
            }
        } else if (io.MouseWheel > 0 || io.MouseWheel < 0) {
            if (orbit_mode_) {
                if (io.MouseWheel > 0)
                    zoom_ -= 0.25f;
                else if (io.MouseWheel < 0)
                    zoom_ += 0.25f;
                if (zoom_ < 0.25f)
                    zoom_ = 0.25f;
                CalculateOrbitPos_();
            } else {
                if (io.MouseWheel > 0)
                    view_pos_ += cam_move_speed_ * cam_fwd_;
                else if (io.MouseWheel < 0)
                    view_pos_ -= cam_move_speed_ * cam_fwd_;
            }
        }

        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
            // Test and handle keyboard input here
            if (ImGui::IsKeyPressed(i) && ImGui::GetActiveID() == 0) {
                if (!orbit_mode_) {
                    if (i == 'Q')
                        view_pos_.y += cam_move_speed_;
                    if (i == 'E')
                        view_pos_.y -= cam_move_speed_;
                    if (i == 'W')
                        view_pos_ += cam_move_speed_ * cam_fwd_;
                    if (i == 'S')
                        view_pos_ -= cam_move_speed_ * cam_fwd_;
                    if (i == 'A')
                        view_pos_ -= glm::normalize(glm::cross(cam_fwd_, cam_up_)) * cam_move_speed_;
                    if (i == 'D')
                        view_pos_ += glm::normalize(glm::cross(cam_fwd_, cam_up_)) * cam_move_speed_;
                }
                if (i == 'H') {
                    view_pos_ = {0.0f, 0.0f, 0.0f};
                    view_center_ = {0.0f, 3.7f, 0.0f};
                    cam_fwd_ = glm::vec3(0.00483705, 0.118993, 0.992883);
                    cam_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
                    yaw_ = 88.5f;
                    pitch_ = 8.88f;
                    lat_ = 0.5437f;
                    lon_ = -0.2505f;
                    zoom_ = 13.5f;
                    CalculateOrbitPos_();
                }
            }
        }
    }
}

void ImGuiOpenGlWindow::AddObject(std::shared_ptr<ImGuiOpenGlObjectBuffer> object)
{
    draw_items_.emplace_back(object);
}

void ImGuiOpenGlWindow::DeleteObject(const std::string &object_name)
{
    for (int i = 0; i < draw_items_.size(); i++) {
        if (draw_items_.at(i)->obj_name == object_name) {
            draw_items_.erase(draw_items_.begin() + i);
        }
    }
}

void ImGuiOpenGlWindow::CalculateOrbitPos_()
{
    orb_rad_ = (cos((lat_ - 0.5f) * pi_));
    view_pos_.x = (cos(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;
    view_pos_.y = (sin((lat_ - 0.5f) * pi_)) * zoom_;
    view_pos_.z = (sin(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;
}

void ImGuiOpenGlWindow::CalculateFlyPos_()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction.y = sin(glm::radians(pitch_));
    direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    cam_fwd_ = glm::normalize(direction);
    glm::vec3 camRight = glm::normalize(glm::cross(cam_fwd_, glm::vec3(0.0f, -1.0f, 0.0f)));
    cam_up_ = glm::normalize(glm::cross(cam_fwd_, camRight));
}

void ImGuiOpenGlWindow::FlyToOrbit_()
{
    view_center_ = {0.0f, 0.0f, 0.0f};
    glm::vec3 pos = {view_pos_.x / zoom_, view_pos_.y / zoom_, view_pos_.z / zoom_};
    pos = glm::normalize(pos);
    lat_ = (asin(pos.y) / pi_) + 0.5f;
    orb_rad_ = (cos((lat_ - 0.5f) * pi_));
    pos.x /= orb_rad_;
    pos.x = (acos(pos.x) / pi_) / 2.0f;
    lon_ = pos.x;
    if (view_pos_.z < 0.0f)
        lon_ *= -1.0f;
    view_pos_.x = (cos(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;
    view_pos_.y = (sin((lat_ - 0.5f) * pi_)) * zoom_;
    view_pos_.z = (sin(lon_ * 2.0f * pi_) * orb_rad_) * zoom_;
}

void ImGuiOpenGlWindow::OrbitToFly_()
{
    glm::vec3 direction;
    glm::vec3 v1;
    v1.x = view_pos_.x;
    v1.y = 0.0f;
    v1.z = view_pos_.z;
    v1 = glm::normalize(v1);
    glm::vec3 v2 = {0.0f, 0.0f, -1.0f};
    float offset = 90.0f;
    if (view_pos_.x < 0)
        offset *= -1.0f;
    yaw_ = glm::degrees(acos(glm::dot(v1, v2))) + offset;
    if (view_pos_.x < 0.0f)
        yaw_ *= -1.0f;
    v1.x = view_pos_.x;
    v1.y = view_pos_.y;
    v1.z = view_pos_.z;
    v1 = glm::normalize(v1);
    glm::vec3 v3 = {0.0f, 1.0f, 0.0f};
    pitch_ = glm::degrees(acos(glm::dot(v1, v3))) - 90.0f;
    if (pitch_ < -89.99f)
        pitch_ = -89.99f;
    else if (pitch_ > 89.99f)
        pitch_ = 89.99f;
    direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction.y = sin(glm::radians(pitch_));
    direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    cam_fwd_ = glm::normalize(direction);
    glm::vec3 camRight = glm::normalize(glm::cross(cam_fwd_, glm::vec3(0.0f, -1.0f, 0.0f)));
    cam_up_ = glm::normalize(glm::cross(cam_fwd_, camRight));
}

bool ImGuiOpenGlWindow::IsMouseInWindow_(ImVec2 min, ImVec2 max)
{
    ImGuiIO &io = ImGui::GetIO();

    if (io.MousePos.x > min.x && io.MousePos.x < max.x) {
        if (io.MousePos.y > min.y && io.MousePos.y < max.y) {
            return true;
        }
    }

    return false;
}

const ImVec4 ImGuiOpenGlWindow::GetViewBgColor() const {
    return view_bg_color_;
}

void ImGuiOpenGlWindow::SetViewBgColor(const ImVec4 &view_bg_color) {
    view_bg_color_ = view_bg_color;
}

float ImGuiOpenGlWindow::GetGridSize() const {
    return grid_size_;
}

void ImGuiOpenGlWindow::SetGridSize(float grid_size) {
    grid_size_ = grid_size;
}

bool ImGuiOpenGlWindow::IsShowGrid() const {
    return show_grid_;
}

void ImGuiOpenGlWindow::SetShowGrid(bool show_grid) {
    show_grid_ = show_grid;
}

const glm::vec3 ImGuiOpenGlWindow::GetViewPos() const {
    return view_pos_;
}

void ImGuiOpenGlWindow::SetViewPos(const glm::vec3 &view_pos) {
    view_pos_ = view_pos;
}

const glm::vec3 ImGuiOpenGlWindow::GetViewCenter() const {
    return view_center_;
}

void ImGuiOpenGlWindow::SetViewCenter(const glm::vec3 &view_center) {
    view_center_ = view_center;
}

float ImGuiOpenGlWindow::GetCamMoveSpeed() const {
    return cam_move_speed_;
}

void ImGuiOpenGlWindow::SetCamMoveSpeed(float cam_move_speed) {
    cam_move_speed_ = cam_move_speed;
}

float ImGuiOpenGlWindow::GetCamFov() const {
    return cam_fov_;
}

void ImGuiOpenGlWindow::SetCamFov(float cam_fov) {
    cam_fov_ = cam_fov;
}

float ImGuiOpenGlWindow::GetYaw() const {
    return yaw_;
}

void ImGuiOpenGlWindow::SetYaw(float yaw) {
    yaw_ = yaw;
}

float ImGuiOpenGlWindow::GetPitch() const {
    return pitch_;
}

void ImGuiOpenGlWindow::SetPitch(float pitch) {
    pitch_ = pitch;
}

float ImGuiOpenGlWindow::GetLat() const {
    return lat_;
}

void ImGuiOpenGlWindow::SetLat(float lat) {
    lat_ = lat;
}

float ImGuiOpenGlWindow::GetLon() const {
    return lon_;
}

void ImGuiOpenGlWindow::SetLon(float lon) {
    lon_ = lon;
}

float ImGuiOpenGlWindow::GetZoom() const {
    return zoom_;
}

void ImGuiOpenGlWindow::SetZoom(float zoom) {
    zoom_ = zoom;
}

bool ImGuiOpenGlWindow::IsOrbitMode() const {
    return orbit_mode_;
}

void ImGuiOpenGlWindow::SetOrbitMode(bool orbit_mode) {
    orbit_mode_ = orbit_mode;
}

const glm::vec3 ImGuiOpenGlWindow::GetCamFwd() const {
    return cam_fwd_;
}

void ImGuiOpenGlWindow::SetCamFwd(const glm::vec3 &cam_fwd) {
    cam_fwd_ = cam_fwd;
}

const glm::vec3 ImGuiOpenGlWindow::GetCamUp() const {
    return cam_up_;
}

void ImGuiOpenGlWindow::SetCamUp(const glm::vec3 &cam_up) {
    cam_up_ = cam_up;
}

bool ImGuiOpenGlWindow::IsUseMsaa() const {
    return use_msaa_;
}

void ImGuiOpenGlWindow::SetUseMsaa(bool use_msaa) {
    use_msaa_ = use_msaa;
}

