# FlowCV Config
message(STATUS "Adding FlowCV CMake Config")

# FlowCV Managers
include_directories(${FLOWCV_PROJ_DIR}/include)
list(APPEND FlowCV_SRC ${FLOWCV_PROJ_DIR}/src/FlowCV_Types.cpp)
list(APPEND FlowCV_SRC ${FLOWCV_PROJ_DIR}/src/FlowCV_Properties.cpp)

# Third Party
include_directories(${FLOWCV_PROJ_DIR}/third-party)
include_directories(${FLOWCV_PROJ_DIR}/third-party/nlohmann)
include_directories(${FLOWCV_PROJ_DIR}/third-party/spdlog/include)
