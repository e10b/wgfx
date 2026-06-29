cmake_minimum_required(VERSION 3.5...3.25)
project(wgfx VERSION 0.1.0 LANGUAGES CXX C)

option(WGFX_BUILD_SDL_BRIDGE "Build the SDL3-to-WebGPU surface bridge" OFF)

add_subdirectory(deps/webgpu)

if(WGFX_BUILD_SDL_BRIDGE)
	add_subdirectory(deps/sdl2webgpu)
endif()

add_library(wgfx STATIC
	src/buffer.cpp
	src/pipeline.cpp
	src/renderpass.cpp
	src/surface.cpp
	src/uniform.cpp
)

target_include_directories(wgfx PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/sdl2webgpu"
)

target_link_libraries(wgfx PUBLIC webgpu)

if(WGFX_BUILD_SDL_BRIDGE)
	target_link_libraries(wgfx PUBLIC sdl2webgpu)
	target_compile_definitions(wgfx PUBLIC WGFX_ENABLE_SDL=1)
endif()

target_compile_features(wgfx PUBLIC cxx_std_17)
