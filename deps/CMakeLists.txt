#cmake_dependent_option(SDL_STATIC "Build a static version of the library" ${SDL_STATIC_DEFAULT} ${SDL_STATIC_AVAILABLE} ON)
set(SDL_STATIC ON CACHE INTERNAL "")
add_subdirectory(sdl3)

add_subdirectory(glm)

add_subdirectory(webgpu)
add_subdirectory(sdl2webgpu)
set(SHADERC_SKIP_TESTS ON CACHE INTERNAL "")
add_subdirectory(fastgltf)
