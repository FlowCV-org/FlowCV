//
// ImGUI OpenCV Class
// Created by Richard Wardlow
//

#ifndef IMGUI_WRAPPER_IMGUI_OPENCV_HPP_
#define IMGUI_WRAPPER_IMGUI_OPENCV_HPP_

#include "opencv2/opencv.hpp"

struct ImGuiOpencvWindowData
{
    int window_width;
    int window_height;
    int tex_width;
    int tex_height;
    int lock_dir;
    int padding;
    float ratio;
    float check_ratio;
};

enum ImOpenCvWindowAspectFlag
{
    ImOpenCvWindowAspectFlag_Free,
    ImOpenCvWindowAspectFlag_LockW,
    ImOpenCvWindowAspectFlag_LockH
};

class ImGuiOpenCvWindow
{
  public:
    ImGuiOpenCvWindow();
    void Update(const char *title, cv::Mat& frame, ImOpenCvWindowAspectFlag flags = ImOpenCvWindowAspectFlag_LockW, int padding = 32);
    ~ImGuiOpenCvWindow();

  protected:

  private:
    ImGuiOpencvWindowData window_data_{};
    unsigned int image_texture_{};
    cv::Mat frame_;
    bool keep_aspect_{};
    bool init_texture_once_{};
    int aspect_select_{};
    int channel_select_{};
    int color_map_select_{};
    float color_scale_{};
    bool set_once_{};
    bool show_color_map_{};
    ImOpenCvWindowAspectFlag window_flags_{};
};

#endif //IMGUI_WRAPPER_IMGUI_OPENCV_HPP_
