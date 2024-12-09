cmake_minimum_required(VERSION 3.5.1)

set(PROJECT_NAME "zhx_test")
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)

# 输入选项
if (WIN32)
    set(PTAG "win64")
    set(SDK_DIR "D:/visualstudio_projects/hangxing-backend")
elseif(UNIX)
    set(dev false CACHE BOOL "build in dev mode")
    set(SDK_DIR "zhx_sdk" CACHE STRING "SDK dir path")
    set(PTAG "x86_64")
else()
    message(CRITICAL "platform not supported")
endif()
message(STATUS "compiling on ${PTAG}")

# 路径变量设置
set(SDK_HEADER_DIR "${SDK_DIR}/win64_jni/include")
set(SDK_LIB_DIR "${SDK_DIR}/win64_jni/output/${PTAG}/lib")
set(MEGFLOW_HEADER_DIR "${SDK_DIR}/win64_jni/3rdparty/include/megflow")
set(GLOG_DIR "${SDK_DIR}/win64_jni/3rdparty/include/${PTAG}")
set(SRC_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
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
    # set(CMAKE_CXX_FLAGS_DEBUG  -g -D_GLIBCXX_DEBUG)
endif()

# sdk library
list(APPEND HEADER_DIRs ${SDK_HEADER_DIR} ${MEGFLOW_HEADER_DIR} ${GLOG_DIR})
list(APPEND LIB_DIRs ${SDK_LIB_DIR})
list(APPEND LIBs zhx_jni)
list(APPEND LIB_DIRs ${SDK_DIR}/win64_jni/3rdparty/libs/${PTAG})

if (UNIX)
    message(STATUS "build on linux")
    # link sdk lib
    list(APPEND LIBs megsearch mmcv_loader mcv_service)

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
    find_package(yaml-cpp CONFIG REQUIRED)
    list(APPEND LIBs yaml-cpp::yaml-cpp)


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
    find_package(GTest CONFIG REQUIRED)
    list(APPEND LIBs GTest::gtest)

else()
    # WIN32
    message(STATUS "build on windows")
    set (CMAKE_SYSTEM_NAME Windows)
    set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
    set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAG_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAG_RELEASE} /MT")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAG_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAG_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
    add_definitions("/Zi /MT")
    add_definitions(-DMEG_COMPILE_TIME=\"${COMPILE_TIME}\")
    add_definitions("/utf-8")
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
    add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
    add_compile_options("/std:c++17")
    add_compile_options(-D_XKEYCHECK_H)
    add_definitions("-D_UNICODE -DUNICODE -D_WINDLL")

    list(APPEND LIBs mcv_service)

    add_subdirectory(thirdparty/spdlog/spdlog-1.14.1)
    add_subdirectory(thirdparty/yaml-cpp/yaml-cpp-0.8.0)
    list(APPEND HEADER_DIRs "thirdparty/nlohmann_json/out/include")

endif()

# sources
file(GLOB_RECURSE SOURCE_FILES
  ${SRC_SOURCE_DIR}/*.cpp
  ${SRC_SOURCE_DIR}/*.c
)
list(APPEND HEADER_DIRs ${SRC_SOURCE_DIR}/include)
link_directories(${LIB_DIRs})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (UNIX)
    add_dependencies(${PROJECT_NAME} ${FAKE_TARGETs})
    list(APPEND LIBs stdc++fs)
else()
    list(APPEND LIBs spdlog::spdlog yaml-cpp::yaml-cpp zhx_jni)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_DIRs})
# target_link_directories(${PROJECT_NAME} PUBLIC ${LIB_DIRs})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBs})

# 设置输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/tmp/${PTAG}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/tmp/${PTAG}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/tmp/${PTAG}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/output/debug/${PTAG}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/output/debug/${PTAG}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/output/debug/${PTAG}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/output/release/${PTAG}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/output/release/${PTAG}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/output/release/${PTAG}/bin)
