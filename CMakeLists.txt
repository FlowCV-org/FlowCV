cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

cmake_policy(SET CMP0068 NEW) # new in 3.9. The NEW behavior of this policy is to ignore the RPATH settings for install_name on macOS.

project(OpenCV_Dataflow_Framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

# When compiling with Release, the console window is not generated
if(WIN32)
  if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows")
  endif(MSVC)
endif(WIN32)

# Options
option(USE_LOCAL_OPENCV_PACKAGE "Use locally OpenCV Package" ON)
option(BUILD_EXAMPLES "Build Examples" ON)
option(BUILD_PLUGINS "Build Plugins" ON)
option(BUILD_EDITOR "Build Editor" ON)
option(BUILD_ENGINE "Build Engine" ON)

# Set Output Path
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
endif()

foreach(OUTPUT_TYPES ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUT_TYPES} OUTPUT_CONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${OUTPUT_TYPES})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OUTPUT_TYPES})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUT_TYPES})
    message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG}}")
endforeach(OUTPUT_TYPES CMAKE_CONFIGURATION_TYPES)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")
set(CMAKE_MINSIZEREL_POSTFIX "s")

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

if(BUILD_PLUGINS)
    # FlowCV Plugin Modules
    add_subdirectory(Plugins)
endif()

if(BUILD_ENGINE)
    # OpenCV DSPatch Standalone Processing Engine
    add_subdirectory(Processing_Engine)
endif()

if(BUILD_EDITOR)
    # OpenCV DSPatch Node Editor UI
    add_subdirectory(Editor_UI)
endif()

if(BUILD_EXAMPLES)
    # Build Examples
    add_subdirectory(Examples/FlowCV_Dataflow_Test)
endif()
