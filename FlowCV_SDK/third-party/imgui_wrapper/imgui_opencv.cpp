//
// ImGUI OpenCV Class
// Created by Richard Wardlow
//

#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include <vector>

ImGuiOpenCvWindow::ImGuiOpenCvWindow()
{
    init_texture_once_ = true;
    set_once_ = true;
    channel_select_ = 0;
    color_map_select_ = 2;
    color_scale_ = 0.15f;
    window_flags_ = ImOpenCvWindowAspectFlag_LockW;
}

ImGuiOpenCvWindow::~ImGuiOpenCvWindow()
{
    if (image_texture_)
        glDeleteTextures(1, &image_texture_);
}

void ImGuiOpenCvWindow::Update(const char *title, cv::Mat &frame, ImOpenCvWindowAspectFlag flags, int padding)
{
    // Set Initial Params Once
    if (set_once_) {
        if ((int)flags == 0)
            keep_aspect_ = false;
        else
            keep_aspect_ = true;
        window_flags_ = flags;
        window_data_.ratio = 1.0f;
        window_data_.check_ratio = 0.0f;
        aspect_select_ = (int)window_flags_;
        set_once_ = false;
    }

    // Set Member Params
    if (!frame.empty()) {
        frame.copyTo(frame_);
        window_data_.window_width = frame_.cols;
        window_data_.window_height = frame_.rows;
        window_data_.check_ratio = (float) window_data_.window_width / (float) window_data_.window_height;
        window_data_.lock_dir = (int) window_flags_;
        window_data_.padding = padding;
    }

    if (keep_aspect_) {
        if (fabs(window_data_.ratio - window_data_.check_ratio) > 0.01f) {
            window_data_.ratio = (float) window_data_.window_width / (float) window_data_.window_height;
        }
    }

    // Begin Viewer UI
    ImGui::Begin(title);
    int winWidth = (int)ImGui::GetWindowWidth();
    int winHeight = (int)ImGui::GetWindowHeight();

    // Aspect Ratio Controls
    const char *aspect_items[] = {"Free", "Width", "Height"};
    ImGui::Text("Aspect Ratio:");
    ImGui::SameLine();
    ImGui::PushItemWidth(80);
    ImGui::Combo("##aspect", &aspect_select_, aspect_items, IM_ARRAYSIZE(aspect_items));
    ImGui::PopItemWidth();
    if (aspect_select_ != (int)window_flags_) {
        if (aspect_select_ == 0)
            keep_aspect_ = false;
        else
            keep_aspect_ = true;
        window_flags_ = (ImOpenCvWindowAspectFlag)aspect_select_;
    }

    // Separator
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    // Show Channel Controls
    const char *chan_items[] = {"RGB", "R", "G", "B", "A", "Lum"};
    ImGui::Text("Channel:");
    ImGui::SameLine();
    ImGui::PushItemWidth(50);
    ImGui::Combo("##chan", &channel_select_, chan_items, IM_ARRAYSIZE(chan_items));
    ImGui::PopItemWidth();

    // 16-Bit (Depth Map) Color Map Option
    if (show_color_map_) {
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        const char *color_map_colors[] = {
            "AUTUMN",
            "BONE",
            "JET",
            "WINTER",
            "RAINBOW",
            "OCEAN",
            "SUMMER",
            "SPRING",
            "COOL",
            "HSV",
            "PINK",
            "HOT",
            "PARULA",
            "MAGMA",
            "INFERNO",
            "PLASMA",
            "VIRIDIS",
            "CIVIDIS",
            "TWILIGHT",
            "TWILIGHT_SHIFTED",
            "TURBO"
        };
        ImGui::Text("Color Map:");
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::Combo("##map", &color_map_select_, color_map_colors, IM_ARRAYSIZE(color_map_colors));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat("##scale", &color_scale_, 0.001f, 0.001f, 10.0f);
        ImGui::PopItemWidth();
    }

    // Aspect Ratio Constraint Handling
    if (keep_aspect_) {
        float curRatio = (float)winWidth / (float)winHeight;
        if (curRatio < (window_data_.ratio - 0.001f) || curRatio > (window_data_.ratio + 0.001f)) {
            if (window_flags_ == ImOpenCvWindowAspectFlag_LockW) {
                winWidth -= (padding);
                if (winWidth < 128)
                    winWidth = 128;
                winHeight = (int) ((float) winWidth / window_data_.ratio);
                ImGui::SetWindowSize(ImVec2((float)(winWidth + padding), (float)(winHeight + (padding * 2))));
            }
            else if (window_flags_ == ImOpenCvWindowAspectFlag_LockH) {
                winHeight -= (padding * 2);
                if (winHeight < 128)
                    winHeight = 128;
                winWidth = (int) ((float) winHeight * window_data_.ratio);
                ImGui::SetWindowSize(ImVec2((float)(winWidth + padding), (float)(winHeight + (padding * 2))));
            }
        }
    }
    else {
        winHeight -= (padding * 2);
        winWidth -= padding;
        if (winWidth < 128)
            winWidth = 128;
        if (winHeight < 128)
            winHeight = 128;
        ImGui::SetWindowSize(ImVec2((float)(winWidth + padding), (float)(winHeight + (padding * 2))));
    }

    // Max sure Image is Divisible by 4
    int mod = winWidth % 4;
    if (mod != 0) {
        winWidth -= mod;
    }
    mod = winHeight % 4;
    if (mod != 0) {
        winHeight -= mod;
    }

    // Resize Input Image to Fit Window
    if (winWidth <= 0)
        winWidth = 1;
    if (winHeight <= 0)
        winHeight = 1;

    window_data_.tex_width = frame_.cols;
    window_data_.tex_height = frame_.rows;

    if (init_texture_once_) {
        glGenTextures(1, &image_texture_);
        glBindTexture(GL_TEXTURE_2D, image_texture_);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
        init_texture_once_ = false;
    }
    else {
        glBindTexture(GL_TEXTURE_2D, image_texture_);
    }

    // Handle Input Image Channel Conversion for Viewer
    show_color_map_ = false;
    if (frame_.type() == CV_8UC3 && frame_.channels() == 3) {
        cv::cvtColor(frame_, frame_, cv::COLOR_BGR2BGRA);
    }
    else if (frame_.type() == CV_8UC1 && frame_.channels() == 1) {
        cv::cvtColor(frame_, frame_, cv::COLOR_GRAY2BGRA);
    }
    else if (frame_.type() == CV_16U || frame_.type() == CV_16UC1 || frame_.type() == CV_16S) {
        if (frame_.channels() == 1) {
            cv::Mat tmp_frame;
            frame_.convertTo(tmp_frame, CV_8UC1, color_scale_);
            normalize(tmp_frame, frame_, 1, 255, cv::NORM_MINMAX, CV_8UC1);
            cvtColor(frame_, frame_, cv::COLOR_GRAY2BGR);
            applyColorMap(frame_, frame_, (cv::ColormapTypes) color_map_select_);
            cvtColor(frame_, frame_, cv::COLOR_BGR2BGRA);
            show_color_map_ = true;
        }
    }
    else if (frame_.type() == CV_32F || frame_.type() == CV_32FC1 || frame_.type() == CV_64F) {
        if (frame_.channels() == 1) {
            cv::Mat tmp_frame;
            frame_.convertTo(tmp_frame, CV_8UC1, color_scale_);
            normalize(tmp_frame, frame_, 1, 255, cv::NORM_MINMAX, CV_8UC1);
            cvtColor(frame_, frame_, cv::COLOR_GRAY2BGR);
            applyColorMap(frame_, frame_, (cv::ColormapTypes) color_map_select_);
            cvtColor(frame_, frame_, cv::COLOR_BGR2BGRA);
            show_color_map_ = true;
        }
    }

    // Channel Selector View Handling
    if (channel_select_ == 0) {            // RGB
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame_.data);
    }
    else if (channel_select_ == 1) {        // Red
        std::vector<cv::Mat> bgr_planes;
        split( frame_, bgr_planes );
        cv::Mat SFrame;
        cv::cvtColor(bgr_planes[2], SFrame, cv::COLOR_GRAY2BGR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, SFrame.data);
    }
    else if (channel_select_ == 2) {        // Green
        std::vector<cv::Mat> bgr_planes;
        split( frame_, bgr_planes );
        cv::Mat SFrame;
        cv::cvtColor(bgr_planes[1], SFrame, cv::COLOR_GRAY2BGR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, SFrame.data);
    }
    else if (channel_select_ == 3) {        // Blue
        std::vector<cv::Mat> bgr_planes;
        split( frame_, bgr_planes );
        cv::Mat SFrame;
        cv::cvtColor(bgr_planes[0], SFrame, cv::COLOR_GRAY2BGR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, SFrame.data);
    }
    else if (channel_select_ == 4) {       // Alpha
        if (frame_.channels() > 3) {
            std::vector<cv::Mat> bgr_planes;
            split(frame_, bgr_planes);
            cv::Mat SFrame;
            cv::cvtColor(bgr_planes[3], SFrame, cv::COLOR_GRAY2BGR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, SFrame.data);
        }
    }
    else if (channel_select_ == 5) {       // Luminance
        cv::Mat SFrame;
        cv::cvtColor(frame_, SFrame, cv::COLOR_BGRA2GRAY);
        cv::cvtColor(SFrame, SFrame, cv::COLOR_GRAY2BGR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_data_.tex_width, window_data_.tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, SFrame.data);
    }

    // Draw Input Image in Viewer
    ImGui::SetCursorPos(ImVec2((((ImGui::GetWindowWidth() - (float)padding) - (float)winWidth) * 0.5f) + (float)padding / 2, (float)padding + 20));
    ImGui::Image((ImTextureID)image_texture_, ImVec2((float)winWidth, (float)winHeight));

    // Color Inspector Tooltip Overlay
    ImGuiIO &io = ImGui::GetIO();
    if (io.KeyCtrl) {
        ImVec2 rect_min = ImGui::GetItemRectMin();
        ImVec2 mouse_uv_coord = io.MousePos;

        mouse_uv_coord.x = (mouse_uv_coord.x - rect_min.x) / (float)winWidth;
        mouse_uv_coord.y = (mouse_uv_coord.y - rect_min.y) / (float)winHeight;
        if (mouse_uv_coord.x >= 0.0f && mouse_uv_coord.y >= 0.0f && mouse_uv_coord.x < 1.0f && mouse_uv_coord.y < 1.0f) {
            ImGui::BeginTooltip();
            ImGui::BeginGroup();
            int x_pick = int(mouse_uv_coord.x * (float)window_data_.tex_width);
            int y_pick = int(mouse_uv_coord.y * (float)window_data_.tex_height);
            int x_org_coord = (int)(mouse_uv_coord.x * (float)window_data_.window_width);
            int y_org_coord = (int)(mouse_uv_coord.y * (float)window_data_.window_height);
            cv::Vec4b color_pick = frame_.at<cv::Vec4b>(y_pick,x_pick);
            ImVec4 color_RGB = ImColor(color_pick[2], color_pick[1], color_pick[0]);
            ImVec4 color_HSV;
            ImGui::ColorConvertRGBtoHSV(color_RGB.x, color_RGB.y, color_RGB.z, color_HSV.x, color_HSV.y, color_HSV.z);
            ImGui::ColorButton("ColorInfo##3c", *(ImVec4*)&color_RGB, ImGuiColorEditFlags_NoBorder, ImVec2(64, 64));
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("Res: %ix%i", window_data_.tex_width, window_data_.tex_height);
            ImGui::Text("U %1.3f V %1.3f", mouse_uv_coord.x, mouse_uv_coord.y);
            ImGui::Text("Coord %i %i", x_org_coord, y_org_coord);
            ImGui::Separator();
            ImGui::Text("R %1i G %1i B %1i", color_pick[2], color_pick[1], color_pick[0]);
            ImGui::Separator();
            ImGui::Text("H %1.3f S %1.3f V %1.3f", color_HSV.x, color_HSV.y, color_HSV.z);
            ImGui::EndGroup();
            ImGui::EndTooltip();
        }
    }
    ImGui::End();

}
