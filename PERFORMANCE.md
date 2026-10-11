# wgfx performance evidence

## Conclusion

For the core operations measured, wgfx is effectively as fast as issuing the
equivalent WebGPU calls directly through wgpu-native. After controlling test
order, no repeatable forwarding, encoding, or queue-submission penalty was
resolved above the measurement noise floor.

This is a scoped empirical claim, not a promise that every workload, backend,
or resource-loading pattern must be identical.

## Test system

- Apple M3 Pro, 14 GPU cores, arm64
- macOS 15.6.1 (24G90)
- AppleClang 17.0.0
- `RelWithDebInfo` (`-O2 -g -DNDEBUG`)
- wgpu-native through WebGPU-distribution commit
  `50010b083e4a1bbdac8f9102602df915fe7404cb`

The executable creates a hidden 64x64 SDL window. Raw and wgfx paths share the
same process, adapter, device, queue, shaders, buffers, textures, pipelines,
bind-group shapes, and workload sizes.

## Method

Each microbenchmark performs ten warm-up batches followed by repeated timed
samples. `std::chrono::steady_clock` records CPU time. Submission and render
comparisons reverse raw/wgfx order on every sample, preventing thermal, cache,
or queue drift from consistently favoring either path. Queue completion occurs
outside CPU submission samples so neither implementation inherits additional
backpressure.

Five complete suite executions were collected. The table averages each
execution's median rather than treating all calls as statistically independent.
The allocation counter observes C++ allocations in the benchmark executable and
static wgfx library, but not allocations internal to wgpu-native, Metal, SDL,
or the operating system.

## Results

| Operation | Raw median | wgfx median | Relative difference |
| --- | ---: | ---: | ---: |
| compute encode, one dispatch | 6.142 us | 6.138 us | -0.1% |
| storage update, 4 KiB | 2.856 us | 2.921 us | +2.3% |
| bind-group recreation | 1.766 us | 1.717 us | -2.8% |
| compute-pipeline creation | 96.925 us | 93.584 us | -3.4% |
| encode + queue submit | 21.321 us | 21.869 us | +2.6% |
| submit + completion | 1.197 ms | 1.215 ms | +1.5% |
| render encode, one draw | 17.667 us | 16.708 us | -5.4% |
| render encode, 100 draws | 33.717 us | 34.108 us | +1.2% |
| render encode, 1,000 draws | 154.442 us | 158.950 us | +2.9% |

The original fixed-order encode-and-submit measurement suggested +22.4% wgfx
overhead. Against canonical wgfx, with execution order reversed on every sample
and 160 samples per implementation, the result is +2.6%. Compute encoding alone
is -0.1%, and submit plus completion is +1.5%. Order-controlled large render
workloads remain within 2.9%. These results resolve no material steady-state
wrapper penalty.

Profiling the canonical source also exposed unconditional diagnostic output in
storage, bind-group, and pipeline creation, plus handles overwritten during
bind-group recreation and pipeline initialization. The benchmark branch removes
the hot-path printing and releases replaced bind groups, layouts, and temporary
pipeline layouts. The table reports the corrected implementation.

Negative values are not treated as acceleration. Creation tests are noisier and
are not representative of a steady-state frame loop. A 256 KiB storage-buffer
create/destroy test retains a 22.3% wrapper cost; it is a resource-lifecycle
operation rather than a per-frame forwarding call and is reported in the CSV.

## Application exercise: wDoom

wDoom provides a real consumer workload. It uses wgfx to drive a WebGPU compute
implementation of DOOM wall-column rasterization, exercising persistent storage
buffers, updates, bind groups, compute dispatch, submission, and presentation.

A deterministic software-renderer reference and GPU buffer readback validated
all four embedded Ultimate DOOM demos: 8,738 eligible frames and 559,232,000
indexed pixels were compared with zero pixel mismatches, zero readback failures,
and zero callbacks pending at completion. This is correctness and integration
evidence; it is not substituted for the controlled raw-versus-wgfx timings.

## Reproduce

Configure with the benchmark target enabled, build, and run the executable:

```sh
cmake -S . -B out/build/benchmark -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWGFX_BUILD_BENCHMARKS=ON
cmake --build out/build/benchmark -j
./out/build/benchmark/wgfx_benchmark results.csv
```

Run the entire suite several times and compare per-run medians. The committed
CSV summaries are under `benchmarks/results/`; the typeset technical note is
`docs/wgfx-performance-evidence.tex`.

GPU timestamp queries were unavailable in the requested device feature set, so
the report makes no shader-only duration claim. CPU wall-clock results should
not be reinterpreted as GPU execution times.
