cmake_minimum_required(VERSION 3.0...3.25)
project(wegs VERSION 0.1.0 LANGUAGES CXX C)

#add_subdirectory(deps/sdl)
#add_subdirectory(deps/webgpu)
#add_subdirectory(deps/sdl2webgpu)
#set(SHADERC_SKIP_TESTS ON CACHE INTERNAL "")
#add_subdirectory(deps/shaderc)

add_subdirectory(deps)


file(GLOB_RECURSE src "src/*.cpp")
list(FILTER src EXCLUDE REGEX "src/main.cpp")

add_executable( App ${src} src/main.cpp )
 #"sdl2webgpu/sdl2webgpu.c" "sdl2webgpu/sdl2webgpu.h")

target_link_libraries(App PRIVATE SDL2-static webgpu sdl2webgpu )
target_include_directories(App PRIVATE src "deps/sdl/include")

target_copy_webgpu_binaries(App)


set_target_properties(App PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
#	COMPILE_WARNING_AS_ERROR ON
)