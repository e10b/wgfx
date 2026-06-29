# wgfx

<img src="logo.png" alt="wgfx logo" width="50%">

wgfx is a lightweight C++ WebGPU graphics framework focused on practical rendering and compute workflows.

This branch holds the runnable native examples and their assets. The core library lives in `deps/wgfx`, pinned to the legacy example-support source used by these demos.

## Examples

```bash
git fetch origin examples
git checkout examples
git submodule update --init --recursive
```

This branch includes examples for a rainbow quad, spinning cube, textured cube, compute sphere, compute boids, shadow mapping, and deferred rendering.

## Build

```bash
cmake -S . -B build
cmake --build build
```

The configure step builds the examples with their top-level dependencies from `deps/`.

## Run Examples

```bash
./build/ExampleRainbowQuad
./build/ExampleSpinningCube
./build/ExampleTexturedCube
./build/ExampleComputeSphere
./build/ExampleComputeBoids
./build/ExampleShadowMapping
./build/ExampleDeferredRendering
```

## Licensing

`wgfx` is dual-licensed:
* **Open Source / Non-Commercial:** Licensed under the GNU AGPLv3. If you use `wgfx` in an open-source project, it's 100% free.
* **Commercial:** If you want to use `wgfx` in a closed-source commercial project, game, or proprietary application, the AGPLv3 terms do not apply, and you must acquire a commercial license. Contact `ethan34787@gmail.com` for pricing details.
