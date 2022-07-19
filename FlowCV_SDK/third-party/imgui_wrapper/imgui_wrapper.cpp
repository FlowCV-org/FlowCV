//
// ImGUI Wrapper Class
// Created by Richard Wardlow
//

#include "imgui_wrapper.hpp"
#ifdef __linux__
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

void ImGuiWrapper::GlfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

ImGuiWrapper::ImGuiWrapper()
{
    window_ = nullptr;
    io = nullptr;
    width_ = 640;
    height_ = 480;
    title_ = "Default Window";
    glsl_version_ = "";
    bkg_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.0f);
}

bool ImGuiWrapper::Init(int width, int height, const char *title, ImGuiConfigFlags add_config)
{
    width_ = width;
    height_ = height;
    title_ = title;

    // Setup window
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit())
        return false;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    glsl_version_ = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    glsl_version_ = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    glsl_version_ = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), NULL, NULL);
    if (window_ == nullptr) {
        printf("glfw Create Window Failed\n");
        return false;
    }
#ifdef __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *path;
    if (count != -1) {
        path = dirname(result);
    }
    std::string iconPath = path;
    iconPath += "/Node_Editor_Resource/AppIcon.png";
    if (std::filesystem::exists(iconPath)) {
        GLFWimage images[1];
        images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels
        glfwSetWindowIcon(window_, 1, images);
        stbi_image_free(images[0].pixels);
    }
#endif
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    //glfwSwapInterval(0); // Disable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#ifdef _WIN32
    bool err = gl3wInit() != 0;
#else
    bool err = glfwInit() != 0;
#endif
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGL() == 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
        bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
        bool err = false;
        glbinding::Binding::initialize();
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
        bool err = false;
        glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
    #else
        bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }

    im_context_ = ImGui::CreateContext();
#ifdef IMPLOT_ENABLED
    ImPlot::CreateContext();
#endif
    io = (ImGuiIO *)&ImGui::GetIO();
    cfg_flags_ = io->ConfigFlags;

    cfg_flags_ |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    cfg_flags_ |= ImGuiConfigFlags_DockingEnable;
    cfg_flags_ |= add_config;

    io->ConfigFlags = cfg_flags_;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version_.c_str());


    return true;
}

ImGuiWrapper::~ImGuiWrapper()
{
    GlfwCleanup();
}

ImGuiContext *ImGuiWrapper::GetImGuiCurrentContext()
{
    return im_context_;
}

void ImGuiWrapper::GlfwCleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
#ifdef IMPLOT_ENABLED
    ImPlot::DestroyContext();
#endif
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void ImGuiWrapper::SetBackgroundColor(ImVec4 color)
{
    bkg_color_.x = color.x;
    bkg_color_.y = color.y;
    bkg_color_.z = color.z;
    bkg_color_.w = color.w;
}

bool ImGuiWrapper::ShouldClose()
{
    return glfwWindowShouldClose(window_);
}

void ImGuiWrapper::SetShouldClose(bool val)
{
    glfwSetWindowShouldClose(window_, val);
}

void ImGuiWrapper::PollEvents()
{
    glfwPollEvents();
}

void ImGuiWrapper::Update()
{
    ImGui::Render();
    //int display_w, display_h;
    //glfwGetFramebufferSize(window_, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    glClearColor(bkg_color_.x * bkg_color_.w, bkg_color_.y * bkg_color_.w, bkg_color_.z * bkg_color_.w, bkg_color_.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io != nullptr) {
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    glfwSwapBuffers(window_);
}

void ImGuiWrapper::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
}

void ImGuiWrapper::StartDockSpace(bool add_menu_space)
{
    // Create the docking environment
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 newPos;
    ImVec2 newSize;
    newPos.x = viewport->Pos.x;
    newSize.x = viewport->Size.x;
    if (add_menu_space) {
        newPos.y = viewport->Pos.y + ImGui::GetFrameHeight();
        newSize.y = viewport->Size.y - ImGui::GetFrameHeight();
    }
    else {
        newPos.y = viewport->Pos.y;
        newSize.y = viewport->Size.y;
    }
    ImGui::SetNextWindowPos(newPos);
    ImGui::SetNextWindowSize(newSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockSpaceId = ImGui::GetID("InvisibleWindowDockSpace");

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
}

void ImGuiWrapper::FrameEnd()
{
    ImGui::End();
}

void ImGuiWrapper::SetWindowTitle(const char *title)
{
    glfwSetWindowTitle(window_, title);
}


