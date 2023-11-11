message(STATUS "Adding FlowCV CMake Config")

include_directories(${CMAKE_SOURCE_DIR}/Managers)
include_directories(${CMAKE_SOURCE_DIR}/Utils)
include_directories(${CMAKE_SOURCE_DIR}/FlowCV_SDK/include)
list(APPEND FlowCV_SRC ${CMAKE_SOURCE_DIR}/FlowCV_SDK/src/FlowCV_Types.cpp)
list(APPEND FlowCV_SRC ${CMAKE_SOURCE_DIR}/FlowCV_SDK/src/FlowCV_Properties.cpp)
