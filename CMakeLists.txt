cmake_minimum_required(VERSION 3.21)
project(OpenCV_Dataflow_Framework)

set(CMAKE_CXX_STANDARD 17)

# Options
option(DOWNLOAD_OPENCV_PACKAGE "Download OpenCV Package" ON)
option(BUILD_EXAMPLES "Build Examples" OFF)
option(BUILD_PLUGINS "Build Plugins" OFF)
option(BUILD_EDITOR "Build Editor" OFF)
option(BUILD_ENGINE "Build Engine" OFF)

# Include all modules by default, modify based on your project needs
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiOpenCvConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiGlfwConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiWrapperConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGui3DOpenGLConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiPlotConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiNodeEditorConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ImGuiFileDialogConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/DSPatchConfig.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/FlowCVConfig.cmake)

# JSON
include_directories(${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/nlohmann)

if (BUILD_PLUGINS)
    # FlowCV Plugin Modules
    add_subdirectory(./Plugins/VideoCapture)
    add_subdirectory(./Plugins/VideoWriter)
    add_subdirectory(./Plugins/ImageLoader)
    add_subdirectory(./Plugins/VideoLoader)
    add_subdirectory(./Plugins/ShapeColorizer)
    add_subdirectory(./Plugins/ShapeCounter)
    add_subdirectory(./Plugins/SimpleBlobTracker)
    add_subdirectory(./Plugins/DataOutput)
    add_subdirectory(./Plugins/ImageWriter)


endif()

if (BUILD_ENGINE)
    # OpenCV DSPatch Standalone Processing Engine
    add_subdirectory(Processing_Engine)
endif()

if (BUILD_EDITOR)
    # OpenCV DSPatch Node Editor UI
    add_subdirectory(Editor_UI)
endif()

if (BUILD_EXAMPLES)
    # Build Examples
    add_subdirectory(Examples/FlowCV_Dataflow_Test)
endif()
