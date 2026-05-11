# wgfx

![wgfx logo](logo.png)

wgfx is a lightweight C++ WebGPU graphics framework focused on practical rendering and compute workflows.

It includes:
- A small wrapper layer around WebGPU setup and pipeline primitives
- Native wgfx examples for a rainbow quad, spinning cube, and compute sphere
- A CMake-first layout for straightforward cross-platform builds

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run Examples

```bash
./build/ExampleRainbowQuad
./build/ExampleSpinningCube
./build/ExampleComputeSphere
```

## License

This project is licensed under the GNU Affero General Public License v3.0.
See LICENSE for details.
