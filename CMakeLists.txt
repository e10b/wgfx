cmake_minimum_required(VERSION 3.16...3.25)
project(wgfx VERSION 0.1.0 LANGUAGES CXX)

file(GLOB WGFX_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(wgfx STATIC ${WGFX_SOURCES})
add_library(wgfx::wgfx ALIAS wgfx)

target_include_directories(wgfx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(wgfx PUBLIC cxx_std_17)
set_target_properties(wgfx PROPERTIES
    CXX_EXTENSIONS OFF
)

option(WGFX_BUILD_BENCHMARKS "Build raw WebGPU comparison benchmarks" OFF)
if(WGFX_BUILD_BENCHMARKS)
    if(NOT TARGET webgpu)
        message(FATAL_ERROR "WGFX_BUILD_BENCHMARKS requires a consumer-provided webgpu target")
    endif()
    add_executable(wgfx_benchmark benchmarks/wgfx_benchmark.cpp)
    target_link_libraries(wgfx_benchmark PRIVATE wgfx webgpu)
    target_compile_features(wgfx_benchmark PRIVATE cxx_std_17)
    target_compile_definitions(wgfx_benchmark PRIVATE WEBGPU_BACKEND_WGPU)
endif()
