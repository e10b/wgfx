# Examples

This directory contains the native wgfx examples plus a vendored copy of the upstream WebGPU Samples repository.

Local wgfx examples:

1. `rainbow_quad` shows a colored quad with a vertex buffer and index buffer.
2. `spinning_cube` adds a depth buffer, uniform matrix, and animated transform.
3. `compute_sphere` renders a centered sphere in a compute shader and blits it to a quad.

Build the local targets directly from CMake:

- `ExampleRainbowQuad`
- `ExampleSpinningCube`
- `ExampleComputeSphere`

The upstream sample collection lives in `examples/webgpu-samples` and keeps its own `LICENSE.txt`.