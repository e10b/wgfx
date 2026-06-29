cmake_minimum_required(VERSION 3.5...3.25)
project(wgfx_examples VERSION 0.1.0 LANGUAGES CXX C)

option(DEV_MODE "Set up development helper settings" ON)

set(WGFX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/wgfx")

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)

add_subdirectory(deps/sdl3)
add_subdirectory(deps/glm)
add_subdirectory("${WGFX_ROOT}/deps/webgpu" "${CMAKE_BINARY_DIR}/webgpu")
add_subdirectory("${WGFX_ROOT}/deps/sdl2webgpu" "${CMAKE_BINARY_DIR}/sdl2webgpu")
add_subdirectory(deps/fastgltf)

file(GLOB_RECURSE wgfx_src CONFIGURE_DEPENDS "${WGFX_ROOT}/src/*.cpp")

set(helper_support_src
	helper/context.cpp
	helper/clock.cpp
	helper/stb_image_impl.cpp
)

function(add_example_target target_name source_file resource_dir)
	get_filename_component(_wgfx_example_res "${resource_dir}" ABSOLUTE)
	add_executable(${target_name} ${helper_support_src} ${wgfx_src} ${source_file})
	target_compile_definitions(${target_name} PRIVATE RESOURCE_DIR="${_wgfx_example_res}")
	target_include_directories(${target_name} PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${resource_dir}"
		"${WGFX_ROOT}/src"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/sdl3/include"
		"${WGFX_ROOT}/deps/sdl2webgpu"
		"${WGFX_ROOT}/deps"
	)
	target_link_libraries(${target_name} PRIVATE webgpu sdl2webgpu SDL3-static glm::glm fastgltf::fastgltf)
	target_copy_webgpu_binaries(${target_name})
	set_target_properties(${target_name} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)
endfunction()

if(NOT EMSCRIPTEN)
	add_example_target(ExampleRainbowQuad ${CMAKE_CURRENT_SOURCE_DIR}/examples/rainbow_quad/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/rainbow_quad)
	add_example_target(ExampleSpinningCube ${CMAKE_CURRENT_SOURCE_DIR}/examples/spinning_cube/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/spinning_cube)
	add_example_target(ExampleComputeSphere ${CMAKE_CURRENT_SOURCE_DIR}/examples/compute_sphere/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/compute_sphere)
	add_example_target(ExampleRotatingCube ${CMAKE_CURRENT_SOURCE_DIR}/examples/rotatingCube/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/rotatingCube)
	add_example_target(ExampleTexturedCube ${CMAKE_CURRENT_SOURCE_DIR}/examples/texturedCube/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/texturedCube)
	add_example_target(ExampleComputeBoids ${CMAKE_CURRENT_SOURCE_DIR}/examples/computeBoids/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/computeBoids)
	add_example_target(ExampleShadowMapping ${CMAKE_CURRENT_SOURCE_DIR}/examples/shadowMapping/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/shadowMapping)
	add_example_target(ExampleDeferredRendering ${CMAKE_CURRENT_SOURCE_DIR}/examples/deferredRendering/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/examples/deferredRendering)
	target_sources(ExampleDeferredRendering PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui_draw.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui_tables.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui_widgets.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_sdl3.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_wgpu.cpp
	)
	target_include_directories(ExampleDeferredRendering PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
		${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends
	)
	target_compile_definitions(ExampleDeferredRendering PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU SDL_ENABLE_OLD_NAMES)
endif()
