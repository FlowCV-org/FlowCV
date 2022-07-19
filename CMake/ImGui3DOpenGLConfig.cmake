message(STATUS "Adding ImGUI 3D OpenGL CMake Config")

set(IMGUI_3D_OPENGL_DIR ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/imgui_wrapper)
include_directories(${IMGUI_3D_OPENGL_DIR})
include_directories(${IMGUI_3D_OPENGL_DIR}/gl_helpers)

list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_3D_OPENGL_DIR}/imgui_3d_opengl.cpp")
list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_3D_OPENGL_DIR}/gl_helpers/VBO.cpp")
list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_3D_OPENGL_DIR}/gl_helpers/VAO.cpp")
list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_3D_OPENGL_DIR}/gl_helpers/EBO.cpp")
