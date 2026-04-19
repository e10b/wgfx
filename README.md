# wgfx

Minimal WebGPU graphics library.

This repository intentionally contains only the reusable graphics layer under `src/`.
Application/game code and third-party dependency orchestration belong in consumer repositories (for example `voxel`).

## CMake

```cmake
add_subdirectory(deps/wgfx)
target_link_libraries(your_target PRIVATE wgfx::wgfx)
```
