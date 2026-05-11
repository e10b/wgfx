cmake_minimum_required(VERSION 3.5...3.25)
project(wegs VERSION 0.1.0 LANGUAGES CXX C)

#add_subdirectory(deps/sdl)
#add_subdirectory(deps/webgpu)
#add_subdirectory(deps/sdl2webgpu)
#set(SHADERC_SKIP_TESTS ON CACHE INTERNAL "")
#add_subdirectory(deps/shaderc)
option(DEV_MODE "Set up development helper settings" ON)

add_subdirectory(deps)

file(GLOB_RECURSE srcc "src/*.cpp") #intriguing..

set(helper_support_src
	helper/context.cpp
	helper/clock.cpp
	helper/stb_image_impl.cpp
)

function(add_example_target target_name source_file resource_dir)
	add_executable(${target_name} ${helper_support_src} ${srcc} ${source_file})
	target_compile_definitions(${target_name} PRIVATE RESOURCE_DIR="${resource_dir}")
	target_include_directories(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${resource_dir}" src "deps/sdl3/include" "deps")
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
endif()