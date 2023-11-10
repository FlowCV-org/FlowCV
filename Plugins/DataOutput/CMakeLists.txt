project(DataOutput)

include(${CMAKE_SOURCE_DIR}/CMake/asio2SetupConfig.cmake)

LIST(APPEND SER_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/serial_enumerator.cpp)

if(WIN32)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win)
        LIST(APPEND SER_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/win/List_Serial_Ports_Win.cpp)
        LIST(APPEND SER_ENUM_LIB setupapi)
endif()

if(UNIX AND NOT APPLE)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
        LIST(APPEND SER_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/linux/List_Serial_Ports_Linux.cpp)
endif()

if(APPLE)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(FOUNDATION_LIBRARY Foundation)
        set(MAC_SER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/macos/serial)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/macos)
        include_directories(${MAC_SER_DIR}/include)
        LIST(APPEND SER_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/macos/List_Serial_Ports_Mac.cpp)
        set(serial_SRCS
                ${MAC_SER_DIR}/src/serial.cc
                ${MAC_SER_DIR}/include/serial/serial.h
                ${MAC_SER_DIR}/include/serial/v8stdint.h
        )
        list(APPEND serial_SRCS ${MAC_SER_DIR}/src/impl/unix.cc)
        list(APPEND serial_SRCS ${MAC_SER_DIR}/src/impl/list_ports/list_ports_osx.cc)
endif()

set(OSC_DIR ${CMAKE_SOURCE_DIR}/third-party/osc)
include_directories(${OSC_DIR})
file(GLOB OSC_SRC ${OSC_DIR}/*.cpp)

# UDP Send
add_library(
        UdpSend SHARED
        UDP_Send.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
)
target_link_libraries(
        UdpSend
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
        spdlog::spdlog
)

if(WIN32)
        set_target_properties(UdpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
        set_target_properties(UdpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_RPATH "${ORIGIN}"
                BUILD_WITH_INSTALL_RPATH ON
        )
elseif(APPLE)
        set_target_properties(UdpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_NAME_DIR "${ORIGIN}"
                BUILD_WITH_INSTALL_NAME_DIR ON
        )
endif()

# TCP Send
add_library(
        TcpSend SHARED
        TCP_Send.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
)
target_link_libraries(
        TcpSend
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
        spdlog::spdlog
)

if(WIN32)
        set_target_properties(TcpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
        set_target_properties(TcpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_RPATH "${ORIGIN}"
                BUILD_WITH_INSTALL_RPATH ON
        )
elseif(APPLE)
        set_target_properties(TcpSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_NAME_DIR "${ORIGIN}"
                BUILD_WITH_INSTALL_NAME_DIR ON
        )
endif()

# Serial Send
add_library(
        SerialSend SHARED
        Serial_Send.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
        ${SER_ENUM_SRC}
        ${serial_SRCS}
)

if(APPLE)
        target_link_libraries(
                SerialSend
                ${IMGUI_LIBS}
                ${OpenCV_LIBS}
                ${SER_ENUM_LIB}
                ${FOUNDATION_LIBRARY}
                ${IOKIT_LIBRARY}
                spdlog::spdlog
        )
else()
        target_link_libraries(
                SerialSend
                ${IMGUI_LIBS}
                ${OpenCV_LIBS}
                ${SER_ENUM_LIB}
                spdlog::spdlog
        )
endif()

if(WIN32)
        set_target_properties(SerialSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
        set_target_properties(SerialSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_RPATH "${ORIGIN}"
                BUILD_WITH_INSTALL_RPATH ON
        )
elseif(APPLE)
        set_target_properties(SerialSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_NAME_DIR "${ORIGIN}"
                BUILD_WITH_INSTALL_NAME_DIR ON
        )
endif()

# OSC Send
add_library(
        OscSend SHARED
        OSC_Send.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
        ${OSC_SRC}
)
target_link_libraries(
        OscSend
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
        spdlog::spdlog
)

if(WIN32)
        set_target_properties(OscSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
        set_target_properties(OscSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_RPATH "${ORIGIN}"
                BUILD_WITH_INSTALL_RPATH ON
        )
elseif(APPLE)
        set_target_properties(OscSend
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_NAME_DIR "${ORIGIN}"
                BUILD_WITH_INSTALL_NAME_DIR ON
        )
endif()

# CSV File
add_library(
        CsvFile SHARED
        CSV_File.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
)
target_link_libraries(
        CsvFile
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
)

if(WIN32)
        set_target_properties(CsvFile
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
        set_target_properties(CsvFile
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_RPATH "${ORIGIN}"
                BUILD_WITH_INSTALL_RPATH ON
        )
elseif(APPLE)
        set_target_properties(CsvFile
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
                SUFFIX ".fp"
                INSTALL_NAME_DIR "${ORIGIN}"
                BUILD_WITH_INSTALL_NAME_DIR ON
        )
endif()