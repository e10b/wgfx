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
