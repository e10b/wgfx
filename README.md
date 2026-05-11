# wgfx

![wgfx logo](media/logo.png)

wgfx is a lightweight C++ WebGPU graphics framework focused on practical rendering and compute workflows.

This branch holds the library (`src/`), dependencies (`deps/`), and CMake wiring. Runnable C++ samples are maintained on the **`examples`** branch so the default clone stays small.

## Screenshots

![wgfx screenshot 1](media/screenshot1.png)

![wgfx screenshot 2](media/screenshot4.gif)

![wgfx screenshot 3](media/screenshot3.gif)

## Examples branch

```bash
git fetch origin examples
git checkout examples
```

There you will find `ExampleRainbowQuad`, `ExampleSpinningCube`, and `ExampleComputeSphere` plus their WGSL and small helpers under `examples/`.

## Build

```bash
cmake -S . -B build
cmake --build build
```

On this branch the configure step still pulls in `deps/` (SDL3, WebGPU, etc.). Sample executables are only added when the `examples/` tree is present.

## License

This project is licensed under the GNU Affero General Public License v3.0.
See LICENSE for details.
