cmake_minimum_required(VERSION 3.0...3.25)
project(wegs VERSION 0.1.0 LANGUAGES CXX C)

#add_subdirectory(deps/sdl)
#add_subdirectory(deps/webgpu)
#add_subdirectory(deps/sdl2webgpu)
#set(SHADERC_SKIP_TESTS ON CACHE INTERNAL "")
#add_subdirectory(deps/shaderc)
option(DEV_MODE "Set up development helper settings" ON)

add_subdirectory(deps)


file(GLOB_RECURSE src "src/*.cpp")
list(FILTER src EXCLUDE REGEX "src/main.cpp")

add_executable( App ${src} src/main.cpp )
 #"sdl2webgpu/sdl2webgpu.c" "sdl2webgpu/sdl2webgpu.h")

 if(DEV_MODE)
 # In dev mode, we load resources from the source tree, so that when we
 # dynamically edit resources (like shaders), these are correctly
 # versionned.
 target_compile_definitions(App PRIVATE
	 RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/res"
 )
else()
 # In release mode, we just load resources relatively to wherever the
 # executable is launched from, so that the binary is portable
 target_compile_definitions(App PRIVATE
	 RESOURCE_DIR="./res"
 )
endif()

target_link_libraries(App PRIVATE webgpu sdl2webgpu SDL3-static)
target_include_directories(App PRIVATE src "deps/sdl3/include")

target_copy_webgpu_binaries(App)


set_target_properties(App PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
#	COMPILE_WARNING_AS_ERROR ON
)