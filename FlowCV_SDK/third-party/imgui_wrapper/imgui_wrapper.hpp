//
// ImGUI Wrapper Class
// Created by Richard Wardlow
//

#ifndef IMGUI_WRAPPER_IMGUI_WRAPPER_HPP_
#define IMGUI_WRAPPER_IMGUI_WRAPPER_HPP_
#define IMGUI_DEFINE_MATH_OPERATORS
#include <cstdio>
#include <cstring>
#include <string>
#include "imgui.h"
#ifdef __linux__
#include <filesystem>
#include <libgen.h>
#include <unistd.h>
#include <climits>
#endif
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_instance_helper.hpp>

#ifdef IMPLOT_ENABLED
    #include "implot.h"
#endif

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
    // About Desktop OpenGL function loaders:
    //  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
    //  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
    //  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#ifdef _WIN32
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#else
#include <GLFW/glfw3.h>            // Initialize with gl3wInit()
#endif
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    #include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    #include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    #define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
    #include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
    #include <glbinding/gl/gl.h>
    using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    #define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
    #include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
    #include <glbinding/gl/gl.h>
    using namespace gl;
#else
    #include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class ImGuiWrapper
{
  public:
    ImGuiWrapper();
    ~ImGuiWrapper();
    static void GlfwErrorCallback(int error, const char* description);
    bool Init(int width, int height, const char *title, ImGuiConfigFlags add_config = ImGuiConfigFlags_None);
    void SetBackgroundColor(ImVec4 color);
    void SetWindowTitle(const char *title);
    bool ShouldClose();
    void SetShouldClose(bool val);
    static void PollEvents();
    ImGuiContext *GetImGuiCurrentContext();
    static void StartDockSpace(bool add_menu_space = false);
    void Update();
    static void NewFrame();
    static void FrameEnd();

  protected:
    void GlfwCleanup();

  private:
    GLFWwindow *window_{};
    ImGuiIO* io{};
    ImGuiContext *im_context_{};
    int width_{};
    int height_{};
    std::string title_{};
    ImVec4 bkg_color_{};
    ImGuiConfigFlags cfg_flags_{};
    std::string glsl_version_{};
};

#endif //IMGUI_WRAPPER_IMGUI_WRAPPER_HPP_
