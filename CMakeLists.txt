cmake_minimum_required(VERSION 3.5.1)

set(PROJECT_NAME "cmake_template")
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)

# 输入选项
set(dev false CACHE BOOL "build in dev mode")

# 路径变量设置
set(SRC_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

# thirdparty路径
set(THIRDPARTY_DIR "${PROJECT_SOURCE_DIR}/thirdparty")
set(NLOHMANN_JSON_DIR "${THIRDPARTY_DIR}/nlohmann_json")
set(NLOHMANN_JSON_OUTPUT "${THIRDPARTY_DIR}/nlohmann_json/out")
set(SPDLOG_DIR "${THIRDPARTY_DIR}/spdlog")
set(SPDLOG_OUT "${THIRDPARTY_DIR}/spdlog/out")
set(GTEST_DIR "${THIRDPARTY_DIR}/gtest")
set(GTEST_OUT "${THIRDPARTY_DIR}/gtest/out")
set(YAML_CPP_DIR "${THIRDPARTY_DIR}/yaml-cpp")
set(YAML_CPP_OUT "${THIRDPARTY_DIR}/yaml-cpp/out")

# 设置cmake config搜索路径
set(CMAKE_PREFIX_PATH ${GTEST_OUT} ${YAML_CPP_OUT} ${CMAKE_PREFIX_PATH})

# enable clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${dev})
    message(STATUS "build in dev model")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# nlohmann_json 编译
add_custom_target(NLO_JSON
COMMAND OUT=${NLOHMANN_JSON_OUTPUT} make
WORKING_DIRECTORY ${NLOHMANN_JSON_DIR}
)
list(APPEND FAKE_TARGETs NLO_JSON)
list(APPEND HEADER_DIRs ${NLOHMANN_JSON_OUTPUT}/include)

# yaml-cpp 编译
add_custom_target(YAML_CPP
COMMAND OUT=${YAML_CPP_OUT} make
WORKING_DIRECTORY ${YAML_CPP_DIR}
)
list(APPEND FAKE_TARGETs YAML_CPP)
list(APPEND HEADER_DIRs ${YAML_CPP_OUT}/include)
list(APPEND LIB_DIRs ${YAML_CPP_OUT}/lib)
list(APPEND LIBs yaml-cpp)


# spdlog 编译
add_custom_target(SPDLOG
COMMAND OUT=${SPDLOG_OUT} make
WORKING_DIRECTORY ${SPDLOG_DIR}
)
list(APPEND FAKE_TARGETs SPDLOG)
list(APPEND HEADER_DIRs ${SPDLOG_OUT}/include)
list(APPEND LIB_DIRs ${SPDLOG_OUT}/lib)
list(APPEND LIBs spdlog)

# gtest 编译
add_custom_target(GTEST
COMMAND OUT=${GTEST_OUT} make
WORKING_DIRECTORY ${GTEST_DIR}
)
list(APPEND FAKE_TARGETs GTEST)
list(APPEND LIB_DIRs ${GTEST_OUT}/lib)
list(APPEND HEADER_DIRs ${GTEST_OUT}/include)
list(APPEND LIBs gtest)

# sources
file(GLOB_RECURSE SOURCE_FILES
  ${SRC_SOURCE_DIR}/*.cpp
  ${SRC_SOURCE_DIR}/*.c
)
list(APPEND HEADER_DIRs ${SRC_SOURCE_DIR}/include)
link_directories(${LIB_DIRs})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_dependencies(${PROJECT_NAME} ${FAKE_TARGETs})
list(APPEND LIBs stdc++fs)

target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_DIRs})
# target_link_directories(${PROJECT_NAME} PUBLIC ${LIB_DIRs})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBs})

