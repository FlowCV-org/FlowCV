//
// Plugin DepthViewer3D
//

#include "depth_viewer_3D.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

// Vertex Shader source code
static const char *vtxShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "layout (location = 2) in vec3 aNormal;\n"
    "out vec4 color;\n"
    "out vec3 Normal;\n"
    "out vec3 viewDir;\n"
    "out float shade;\n"
    "uniform float isShade;\n"
    "uniform float size;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "   vec4 vd = vec4(0.0, 0.0, -1.0, 1.0) * view;\n"
    "   gl_Position = proj * view * model * vec4(size * aPos.x, size * aPos.y, size * aPos.z, 1.0);\n"
    "   color = aColor;\n"
    "   Normal = aNormal;\n"
    "   viewDir = vec3(vd.x, vd.y, vd.z);\n"
    "   shade = isShade;\n"
    "}\0";
// Fragment Shader source code
static const char *fragShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 color;\n"
    "in vec3 Normal;\n"
    "in vec3 viewDir;\n"
    "in float shade;\n"
    "void main()\n"
    "{\n"
    "   float ambient = 0.10f;\n"
    "   vec3 norm = normalize(Normal);\n"
    "   vec3 lightDirection = normalize(viewDir);\n"
    "   float diffuse = max(dot(norm, lightDirection), 0.0f);\n"
    "   if (shade > 0.5) {diffuse = 1.0f; ambient = 0.0f;}\n"
    "   FragColor = color * (diffuse + ambient);\n"
    "}\n\0";

namespace DSPatch::DSPatchables
{

DepthViewer3D::DepthViewer3D() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Depth_Viewer_3D");
    SetComponentCategory_(DSPatch::Category::Category_Views);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 3 inputs
    SetInputCount_(3, {"color", "depth", "intrin"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 0 outputs
    SetOutputCount_(0);

    SetEnabled(true);

    ogl_init_ = false;
    pnt_init_ = false;
    diff_shading_ = true;
    has_intrinsic_ = false;
    diff_color_ = {0.75f, 0.75f, 0.75f, 1.0f};
    no_intrinsic_hfov_ = 60.0f;
    pos_offset_.x = 0.0f;
    pos_offset_.y = -2.5f;
    pos_offset_.z = 0.0f;
    pntCloudObj = std::make_shared<ImGuiOpenGlObjectBuffer>();
    pntCloudObj->draw_point_size = 1.5f;
    pntCloudObj->obj_name = "PointCloud" + std::to_string(GetInstanceCount());
}

void DepthViewer3D::InitOpenGLWin_()
{

    ogl_win_ = std::make_shared<ImGuiOpenGlWindow>();

    if (ogl_win_->Init()) {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vtxShaderSource, nullptr);
        glCompileShader(vertexShader);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragShaderSource, nullptr);
        glCompileShader(fragmentShader);
        pntCloudObj->shader = glCreateProgram();
        glAttachShader(pntCloudObj->shader, vertexShader);
        glAttachShader(pntCloudObj->shader, fragmentShader);
        glLinkProgram(pntCloudObj->shader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        ogl_init_ = true;
    }
}

void DepthViewer3D::InitPointCloud_()
{
    if (!depth_frame_.empty()) {
        ogl_win_->DeleteObject(pntCloudObj->obj_name);

        if (!pntCloudObj->vao)
            pntCloudObj->vao = std::make_shared<VAO>();
        if (!pntCloudObj->vbo)
            pntCloudObj->vbo = std::make_shared<VBO>();
        int pW = depth_frame_.cols;
        int pH = depth_frame_.rows;

        pntCloudVerts.clear();
        for (int y = 0; y < pH; y++) {
            for (int x = 0; x < pW; x++) {
                colorVertNorm cv{};
                pntCloudVerts.emplace_back(cv);
            }
        }

        glUseProgram(pntCloudObj->shader);
        pntCloudObj->vao->Set();
        pntCloudObj->vao->Bind();
        pntCloudObj->vbo->Set((GLfloat *)pntCloudVerts.data(), (GLsizei)(pntCloudVerts.size() * sizeof(colorVertNorm)), true);
        pntCloudObj->vao->LinkAttrib(pntCloudObj->vbo, 0, 3, GL_FLOAT, 10 * sizeof(float), nullptr);
        pntCloudObj->vao->LinkAttrib(pntCloudObj->vbo, 1, 4, GL_FLOAT, 10 * sizeof(float), (void *)(3 * sizeof(float)));
        pntCloudObj->vao->LinkAttrib(pntCloudObj->vbo, 2, 3, GL_FLOAT, 10 * sizeof(float), (void *)(7 * sizeof(float)));
        pntCloudObj->vao->Unbind();
        pntCloudObj->vbo->Unbind();

        pntCloudObj->obj_type = GL_POINTS;
        pntCloudObj->elem_size = (GLsizei)pntCloudVerts.size();
        ogl_win_->AddObject(pntCloudObj);

        pnt_init_ = true;
    }
}

void DepthViewer3D::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    auto inColor = inputs.GetValue<cv::Mat>(0);
    auto inDepth = inputs.GetValue<cv::Mat>(1);
    auto inIntrinsic = inputs.GetValue<nlohmann::json>(2);
    if (!inDepth) {
        return;
    }

    std::lock_guard<std::mutex> lck(io_mutex_);
    if (!inDepth->empty()) {

        cv::flip(*inDepth, depth_frame_, 1);

        if (!ogl_init_ && !pnt_init_)
            return;

        if (inIntrinsic)
            intrinsic_data_ = *inIntrinsic;
        else
            intrinsic_data_.clear();

        bool has_color = false;

        if (inColor) {
            if (!inColor->empty()) {
                has_color = true;
                cv::flip(*inColor, color_frame_, 1);
                if (color_frame_.size != depth_frame_.size)
                    cv::resize(color_frame_, color_frame_, cv::Size(depth_frame_.cols, depth_frame_.rows), 0, 0, cv::INTER_NEAREST);
            }
        }

        int pW = depth_frame_.cols;
        int pH = depth_frame_.rows;
        float fx, fy, ppx, ppy;
        bool found_intrinsics = false;
        if (!intrinsic_data_.empty()) {  // If intrinsic data exists fill in values
            if (intrinsic_data_.contains("data")) {
                if (!intrinsic_data_["data"].empty()) {
                    if (intrinsic_data_["data"][0].contains("intrinsics")) {
                        if (intrinsic_data_["data"][0]["intrinsics"].contains("depth")) {
                            fx = intrinsic_data_["data"][0]["intrinsics"]["depth"]["fx"].get<float>();
                            fy = intrinsic_data_["data"][0]["intrinsics"]["depth"]["fy"].get<float>();
                            ppx = intrinsic_data_["data"][0]["intrinsics"]["depth"]["ppx"].get<float>();
                            ppy = intrinsic_data_["data"][0]["intrinsics"]["depth"]["ppy"].get<float>();
                            found_intrinsics = true;
                        }
                    }
                }
            }
        }

        if (found_intrinsics)
            has_intrinsic_ = true;
        else
            has_intrinsic_ = false;

        if (!has_intrinsic_) {  // If no intrinsic data create best guess
            const float pi = 3.1415926535897932384626433832795f;
            float aspectRatio = (float)pH / (float)pW;
            float vFov = 2.0f * atan(tan((no_intrinsic_hfov_ * pi / 180.0f) / 2.0f) * aspectRatio);
            ppx = (float)pW / 2.0f;
            ppy = (float)pH / 2.0f;
            fx = ppx / tan(no_intrinsic_hfov_ * 0.5f * pi / 180.0f);
            fy = ppy / tan(vFov * 0.5f);
        }

        if (!pntCloudVerts.empty()) {
            if (pntCloudVerts.size() == pW * pH) {
                for (int y = 0; y < pH; y++) {
                    for (int x = 0; x < pW; x++) {
                        float depthValue = (float)depth_frame_.at<uint16_t>(y, x) * 0.01f;
                        if (depthValue > 0) {
                            if (y == 0) {
                                pntCloudVerts.at((y * pW) + x).x = (((float)x - ppx) / fx) * depthValue;
                                pntCloudVerts.at((y * pW) + x).y = -1.0f * ((((float)y - ppy) / fy) * depthValue);
                                pntCloudVerts.at((y * pW) + x).z = depthValue;
                                pntCloudVerts.at((y * pW) + x).x += (-1.0f * pos_offset_.x);
                                pntCloudVerts.at((y * pW) + x).y += (-1.0f * pos_offset_.y);
                                pntCloudVerts.at((y * pW) + x).z += (-1.0f * pos_offset_.z);
                            }
                            if (y < (pH - 1)) {
                                float depthValue2 = (float)depth_frame_.at<uint16_t>(y + 1, x) * 0.01f;
                                pntCloudVerts.at(((y + 1) * pW) + x).x = (((float)x - ppx) / fx) * depthValue2;
                                pntCloudVerts.at(((y + 1) * pW) + x).y = -1.0f * ((((float)(y + 1) - ppy) / fy) * depthValue2);
                                pntCloudVerts.at(((y + 1) * pW) + x).z = depthValue2;
                                pntCloudVerts.at(((y + 1) * pW) + x).x += (-1.0f * pos_offset_.x);
                                pntCloudVerts.at(((y + 1) * pW) + x).y += (-1.0f * pos_offset_.y);
                                pntCloudVerts.at(((y + 1) * pW) + x).z += (-1.0f * pos_offset_.z);
                            }
                            if (has_color) {
                                cv::Vec3b val = color_frame_.at<cv::Vec3b>(y, x);
                                pntCloudVerts.at((y * pW) + x).r = (float)val[2] / 255.0f;
                                pntCloudVerts.at((y * pW) + x).g = (float)val[1] / 255.0f;
                                pntCloudVerts.at((y * pW) + x).b = (float)val[0] / 255.0f;
                                pntCloudVerts.at((y * pW) + x).a = 1.0f;
                            }
                            else {
                                pntCloudVerts.at((y * pW) + x).r = diff_color_.x;
                                pntCloudVerts.at((y * pW) + x).g = diff_color_.y;
                                pntCloudVerts.at((y * pW) + x).b = diff_color_.z;
                                pntCloudVerts.at((y * pW) + x).a = diff_color_.w;
                            }

                            if (diff_shading_) {
                                int p1, p2, p3;

                                p1 = (y * pW) + x;
                                if (x < (pW - 1))
                                    p2 = (y * pW) + (x + 1);
                                else
                                    p2 = (y * pW) + (x - 1);

                                if (y < (pH - 1))
                                    p3 = ((y + 1) * pW) + x;
                                else
                                    p3 = ((y - 1) * pW) + x;

                                glm::vec3 v1 = {pntCloudVerts.at(p1).x, pntCloudVerts.at(p1).y, pntCloudVerts.at(p1).z};
                                glm::vec3 v2 = {pntCloudVerts.at(p2).x, pntCloudVerts.at(p2).y, pntCloudVerts.at(p2).z};
                                glm::vec3 v3 = {pntCloudVerts.at(p3).x, pntCloudVerts.at(p3).y, pntCloudVerts.at(p3).z};

                                if (y == (pH - 1) && x == (pW - 1)) {
                                    glm::vec3 n1 = glm::cross(v2 - v1, v1 - v3);
                                    pntCloudVerts.at(p1).nx = n1.x;
                                    pntCloudVerts.at(p1).ny = n1.y;
                                    pntCloudVerts.at(p1).nz = n1.z;
                                }
                                else if (y == (pH - 1)) {
                                    glm::vec3 n1 = glm::cross(v1 - v2, v1 - v3);
                                    pntCloudVerts.at(p1).nx = n1.x;
                                    pntCloudVerts.at(p1).ny = n1.y;
                                    pntCloudVerts.at(p1).nz = n1.z;
                                }
                                else if (x == (pW - 1)) {
                                    glm::vec3 n1 = glm::cross(v2 - v1, v3 - v1);
                                    pntCloudVerts.at(p1).nx = n1.x;
                                    pntCloudVerts.at(p1).ny = n1.y;
                                    pntCloudVerts.at(p1).nz = n1.z;
                                }
                                else {
                                    glm::vec3 n1 = glm::cross(v1 - v2, v3 - v1);
                                    pntCloudVerts.at(p1).nx = n1.x;
                                    pntCloudVerts.at(p1).ny = n1.y;
                                    pntCloudVerts.at(p1).nz = n1.z;
                                }
                            }
                            else {
                                pntCloudVerts.at((y * pW) + x).nx = 0.0f;
                                pntCloudVerts.at((y * pW) + x).ny = 0.0f;
                                pntCloudVerts.at((y * pW) + x).nz = 1.0f;
                            }
                        }
                        else {
                            pntCloudVerts.at((y * pW) + x).x = 0.0f;
                            pntCloudVerts.at((y * pW) + x).y = 0.0f;
                            pntCloudVerts.at((y * pW) + x).z = 0.0f;
                            pntCloudVerts.at((y * pW) + x).r = 0.0f;
                            pntCloudVerts.at((y * pW) + x).g = 0.0f;
                            pntCloudVerts.at((y * pW) + x).b = 0.0f;
                            pntCloudVerts.at((y * pW) + x).a = 0.0f;
                        }
                    }
                }
            }
        }
    }
}

bool DepthViewer3D::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }
    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        return true;
    }

    return false;
}

void DepthViewer3D::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    std::lock_guard<std::mutex> lck(io_mutex_);
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!has_intrinsic_) {
            ImGui::DragFloat(CreateControlString("H-FOV", GetInstanceName()).c_str(), &no_intrinsic_hfov_, 1.0f, 1.0f, 200.0f);
            ImGui::Separator();
        }
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Point Size", GetInstanceName()).c_str(), &pntCloudObj->draw_point_size, 0.1f, 0.1f, 100.0f);
        ImGui::Checkbox(CreateControlString("Diffuse Shading", GetInstanceName()).c_str(), &diff_shading_);
        ImGui::ColorEdit3(CreateControlString("Diffuse Color", GetInstanceName()).c_str(), (float *)&diff_color_);
        ImGui::Separator();
        ImGui::Text("Position Offset");
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("X", GetInstanceName()).c_str(), &pos_offset_.x, 0.1f, -500.0f, 500.0f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Y", GetInstanceName()).c_str(), &pos_offset_.y, 0.1f, -500.0f, 500.0f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Z", GetInstanceName()).c_str(), &pos_offset_.z, 0.1f, -500.0f, 500.0f);
    }
    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        if (!ogl_init_) {
            InitOpenGLWin_();
        }
        else {
            if (!depth_frame_.empty()) {
                if (pntCloudVerts.empty() || !pnt_init_) {
                    pnt_init_ = false;
                    InitPointCloud_();
                }
                else if (pntCloudVerts.size() != (depth_frame_.rows * depth_frame_.cols)) {
                    pnt_init_ = false;
                    InitPointCloud_();
                }
            }
            if (pnt_init_) {
                glUseProgram(pntCloudObj->shader);
                if (!diff_shading_)
                    glUniform1f(glGetUniformLocation(pntCloudObj->shader, "isShade"), 1.0f);
                else
                    glUniform1f(glGetUniformLocation(pntCloudObj->shader, "isShade"), 0.0f);
                pntCloudObj->vbo->Bind();
                glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(pntCloudVerts.size() * sizeof(colorVertNorm)), (GLfloat *)pntCloudVerts.data());
                pntCloudObj->vbo->Unbind();
                glUseProgram(0);
            }
            std::string title = "Depth_Viewer_3D_" + std::to_string(GetInstanceCount());
            ogl_win_->Update(title.c_str());
        }
    }
}

std::string DepthViewer3D::GetState()
{
    using namespace nlohmann;

    json state;

    state["diff_shading"] = diff_shading_;
    json jColor;
    jColor["R"] = diff_color_.x;
    jColor["G"] = diff_color_.y;
    jColor["B"] = diff_color_.z;
    state["diff_color"] = jColor;
    state["point_size"] = pntCloudObj->draw_point_size;
    if (!has_intrinsic_)
        state["no_int_hfov"] = no_intrinsic_hfov_;
    json jPosOff;
    jPosOff["x"] = pos_offset_.x;
    jPosOff["y"] = pos_offset_.y;
    jPosOff["z"] = pos_offset_.z;
    state["pos_offset"] = jPosOff;

    if (ogl_win_) {
        state["view_lat"] = ogl_win_->GetLat();
        state["view_lon"] = ogl_win_->GetLon();
        state["yaw"] = ogl_win_->GetYaw();
        state["pitch"] = ogl_win_->GetPitch();
        state["zoom"] = ogl_win_->GetZoom();
        state["fov"] = ogl_win_->GetCamFov();
        state["orbit_mode"] = ogl_win_->IsOrbitMode();
        state["fly_speed"] = ogl_win_->GetCamMoveSpeed();
        state["show_grid"] = ogl_win_->IsShowGrid();
        state["grid_size"] = ogl_win_->GetGridSize();
        state["msaa"] = ogl_win_->IsUseMsaa();
        auto viewPos = ogl_win_->GetViewPos();
        auto viewCenter = ogl_win_->GetViewCenter();
        auto camFwd = ogl_win_->GetCamFwd();
        auto camUp = ogl_win_->GetCamUp();
        auto bgColor = ogl_win_->GetViewBgColor();
        json jPos;
        jPos["x"] = viewPos.x;
        jPos["y"] = viewPos.y;
        jPos["z"] = viewPos.z;
        state["view_pos"] = jPos;
        json jCent;
        jCent["x"] = viewCenter.x;
        jCent["y"] = viewCenter.y;
        jCent["z"] = viewCenter.z;
        state["view_center"] = jCent;
        json jFwd;
        jFwd["x"] = camFwd.x;
        jFwd["y"] = camFwd.y;
        jFwd["z"] = camFwd.z;
        state["cam_fwd"] = jFwd;
        json jUp;
        jUp["x"] = camUp.x;
        jUp["y"] = camUp.y;
        jUp["z"] = camUp.z;
        state["cam_up"] = jUp;
        json jBgCol;
        jBgCol["R"] = bgColor.x;
        jBgCol["G"] = bgColor.y;
        jBgCol["B"] = bgColor.z;
        state["bg_color"] = jBgCol;
    }

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DepthViewer3D::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("diff_shading"))
        diff_shading_ = state["diff_shading"].get<bool>();
    if (state.contains("diff_color")) {
        diff_color_.x = state["diff_color"]["R"].get<float>();
        diff_color_.y = state["diff_color"]["G"].get<float>();
        diff_color_.z = state["diff_color"]["B"].get<float>();
    }
    if (state.contains("pos_offset")) {
        pos_offset_.x = state["pos_offset"]["x"].get<float>();
        pos_offset_.y = state["pos_offset"]["y"].get<float>();
        pos_offset_.z = state["pos_offset"]["z"].get<float>();
    }
    if (state.contains("point_size"))
        pntCloudObj->draw_point_size = state["point_size"].get<float>();

    if (state.contains("no_int_hfov"))
        no_intrinsic_hfov_ = state["no_int_hfov"].get<float>();

    if (!ogl_win_) {
        InitOpenGLWin_();

        if (state.contains("view_lat"))
            ogl_win_->SetLat(state["view_lat"].get<float>());
        if (state.contains("view_lon"))
            ogl_win_->SetLon(state["view_lon"].get<float>());
        if (state.contains("yaw"))
            ogl_win_->SetYaw(state["yaw"].get<float>());
        if (state.contains("pitch"))
            ogl_win_->SetPitch(state["pitch"].get<float>());
        if (state.contains("zoom"))
            ogl_win_->SetZoom(state["zoom"].get<float>());
        if (state.contains("fov"))
            ogl_win_->SetCamFov(state["fov"].get<float>());
        if (state.contains("fly_speed"))
            ogl_win_->SetCamMoveSpeed(state["fly_speed"].get<float>());
        if (state.contains("grid_size"))
            ogl_win_->SetGridSize(state["grid_size"].get<float>());
        if (state.contains("show_grid"))
            ogl_win_->SetShowGrid(state["show_grid"].get<bool>());
        if (state.contains("msaa"))
            ogl_win_->SetUseMsaa(state["msaa"].get<bool>());

        if (state.contains("view_pos")) {
            glm::vec3 viewPos;
            viewPos.x = state["view_pos"]["x"].get<float>();
            viewPos.y = state["view_pos"]["y"].get<float>();
            viewPos.z = state["view_pos"]["z"].get<float>();
            ogl_win_->SetViewPos(viewPos);
        }
        if (state.contains("view_center")) {
            glm::vec3 viewCenter;
            viewCenter.x = state["view_center"]["x"].get<float>();
            viewCenter.y = state["view_center"]["y"].get<float>();
            viewCenter.z = state["view_center"]["z"].get<float>();
            ogl_win_->SetViewCenter(viewCenter);
        }
        if (state.contains("cam_fwd")) {
            glm::vec3 camFwd;
            camFwd.x = state["cam_fwd"]["x"].get<float>();
            camFwd.y = state["cam_fwd"]["y"].get<float>();
            camFwd.z = state["cam_fwd"]["z"].get<float>();
            ogl_win_->SetCamFwd(camFwd);
        }
        if (state.contains("cam_up")) {
            glm::vec3 camUp;
            camUp.x = state["cam_up"]["x"].get<float>();
            camUp.y = state["cam_up"]["y"].get<float>();
            camUp.z = state["cam_up"]["z"].get<float>();
            ogl_win_->SetCamUp(camUp);
        }
        if (state.contains("bg_color")) {
            ImVec4 bgColor;
            bgColor.x = state["bg_color"]["R"].get<float>();
            bgColor.y = state["bg_color"]["G"].get<float>();
            bgColor.z = state["bg_color"]["B"].get<float>();
            bgColor.w = 1.0f;
            ogl_win_->SetViewBgColor(bgColor);
        }
        if (state.contains("orbit_mode"))
            ogl_win_->SetOrbitMode(state["orbit_mode"].get<bool>());
    }
}

}  // End Namespace DSPatch::DSPatchables