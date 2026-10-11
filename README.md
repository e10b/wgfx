# wgfx

Minimal WebGPU graphics library.

This repository intentionally contains only the reusable graphics layer under `src/`.
Application/game code and third-party dependency orchestration belong in consumer repositories (for example `voxel`).

## CMake

```cmake
add_subdirectory(deps/wgfx)
target_link_libraries(your_target PRIVATE wgfx::wgfx)
```

## Benchmarks

Set `WGFX_BUILD_BENCHMARKS=ON` from a consumer which already provides the
`webgpu`, SDL3, and sdl2webgpu targets. The benchmark uses one device and queue
for equivalent raw-WebGPU and wgfx workloads and writes machine-readable CSV.

```sh
cmake -S . -B out/build/wgfx-bench -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWEBGPU_BACKEND=WGPU \
  -DWGFX_BUILD_BENCHMARKS=ON
cmake --build out/build/wgfx-bench --target wgfx_benchmark
./out/build/wgfx-bench/deps/wgfx/wgfx_benchmark wgfx-benchmark.csv
```

See [PERFORMANCE.md](PERFORMANCE.md) for methodology, current measurements,
limitations, profiling commands, and prioritized recommendations.
