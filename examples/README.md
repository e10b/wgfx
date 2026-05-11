# Examples

This directory contains the native wgfx examples.

On the `examples` branch, `vendor/wgfx` is a submodule pinned to the **`main`** branch of this repository (library-only default branch). After cloning, run `git submodule update --init vendor/wgfx` (or `git submodule update --init --recursive` from the repo root) so that path is populated.

Local wgfx examples:

1. `rainbow_quad` shows a colored quad with a vertex buffer and index buffer.
2. `spinning_cube` adds a depth buffer, uniform matrix, and animated transform.
3. `compute_sphere` renders a centered sphere in a compute shader and blits it to a quad.

Build the local targets directly from CMake:

- `ExampleRainbowQuad`
- `ExampleSpinningCube`
- `ExampleComputeSphere`
