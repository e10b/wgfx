# wgfx Examples

This directory contains the native wgfx examples.

The examples branch keeps runnable sample applications here and their shared support code in the repository root. Third-party example dependencies live in the top-level `deps/` folder.

## Examples

1. `rainbow_quad` draws a colored quad with a vertex buffer and index buffer.
2. `spinning_cube` adds a depth buffer, uniform matrix, and animated transform.
3. `texturedCube` builds from the spinning cube shape and samples `Di-3d.png`.
4. `compute_sphere` renders a centered sphere in a compute shader and blits it to a quad.
5. `computeBoids` demonstrates a compute-driven particle flock.
6. `shadowMapping` renders a depth-based shadow example.
7. `deferredRendering` demonstrates the deferred renderer path with ImGui controls.

## Targets

```bash
cmake --build build --target ExampleRainbowQuad
cmake --build build --target ExampleSpinningCube
cmake --build build --target ExampleTexturedCube
cmake --build build --target ExampleComputeSphere
cmake --build build --target ExampleComputeBoids
cmake --build build --target ExampleShadowMapping
cmake --build build --target ExampleDeferredRendering
```
