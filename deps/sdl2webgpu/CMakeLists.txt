if(TARGET SDL3-static)
  set(WGFX_SDL3_TARGET SDL3-static)
elseif(TARGET SDL3::SDL3)
  set(WGFX_SDL3_TARGET SDL3::SDL3)
else()
  message(FATAL_ERROR "sdl2webgpu needs SDL3. Add SDL3 before adding wgfx.")
endif()

add_library(sdl2webgpu STATIC "sdl2webgpu.c" "sdl2webgpu.h")
target_include_directories(sdl2webgpu PUBLIC .)
target_link_libraries(sdl2webgpu PUBLIC ${WGFX_SDL3_TARGET} webgpu)


if (APPLE)
  target_compile_options(sdl2webgpu PRIVATE -x objective-c)
  target_link_libraries(sdl2webgpu PRIVATE "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
endif ()
