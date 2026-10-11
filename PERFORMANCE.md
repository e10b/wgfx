# wgfx performance study

## Result in one sentence

The measured wgfx forwarding and encoding helpers add no consistently
resolvable CPU cost over raw WebGPU at the current noise floor; the important
measured cost is native per-frame synchronization, while the largest source-audit
risks are oversized uniform allocations, transient render-pass allocations,
CPU mip generation, debug logging, and incomplete resource ownership.

This report distinguishes measurements from source-based findings. A negative
"overhead" below is measurement noise/order sensitivity, not evidence that a
wrapper makes WebGPU intrinsically faster.

## Test system

- Apple M3 Pro, 14 GPU cores, arm64
- macOS 15.6.1 (24G90)
- AppleClang 17.0.0
- `RelWithDebInfo` (`-O2 -g -DNDEBUG`)
- wgpu-native through WebGPU-distribution commit
  `50010b083e4a1bbdac8f9102602df915fe7404cb`
- wgfx baseline `d7c4f68a4ae57f4862c8c33a3cf8354cc6e67985`

The executable creates a hidden 64x64 SDL window because wgpu-native on this
Mac did not expose a suitable adapter for a surface-less request. Both paths
then share exactly the same adapter, device, queue, shaders, buffers, textures,
pipelines, bind-group shapes, and workload dimensions.

## Method

Each microbenchmark performs ten warm-up batches followed by 12-120 timed
samples. Very short operations are batched. `std::chrono::steady_clock` records
CPU time. The suite reports mean, median, p95, standard deviation, upload bytes,
and application-visible C++ `new` allocations. Queue-writing and asynchronous
submission tests drain the device *outside* each timed sample so one
implementation does not inherit more queue backpressure than the other.

Five full suite executions were collected. Submission and render comparisons
reverse raw/wgfx order on every sample so thermal, cache, and queue drift cannot
consistently favor either path; submission uses 160 samples per implementation.
The table below averages each execution's reported median. The allocation counter sees C++
allocations in the executable and static wgfx library, but not allocations
inside wgpu-native, Metal, SDL, or the OS.

## CPU results

| Operation | Workload | Raw median | wgfx median | Absolute delta | Relative delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| storage create/destroy | 256 B | 2.083 us | 2.008 us | -0.075 us | -3.6% |
| storage create/destroy | 4 KiB | 2.033 us | 2.025 us | -0.008 us | -0.4% |
| storage update | 256 B | 2.177 us | 2.233 us | +0.056 us | +2.6% |
| storage update | 4 KiB | 2.814 us | 2.898 us | +0.084 us | +3.0% |
| storage update | 256 KiB | 32.164 us | 34.592 us | +2.428 us | +7.5% |
| bind-group recreate | one storage binding | 1.767 us | 1.708 us | -0.059 us | -3.3% |
| compute pipeline create | one storage binding | 116.575 us | 98.259 us | -18.316 us | -15.7% |
| compute encode | one dispatch | 5.763 us | 5.779 us | +0.017 us | +0.3% |
| encode + queue submit | one dispatch | 16.625 us | 16.359 us | -0.266 us | -1.6% |
| submit + completion | one dispatch | 1.194 ms | 1.233 ms | +0.039 ms | +3.3% |
| render encode | one draw | 12.192 us | 11.983 us | -0.209 us | -1.7% |
| render encode | 100 draws | 22.033 us | 22.550 us | +0.517 us | +2.3% |
| render encode | 1,000 draws | 127.008 us | 129.558 us | +2.550 us | +2.0% |

The former fixed-order submission result was +22.4%. With per-sample reversed
ordering it becomes -1.6%; the five individual run deltas span both signs, from
-3.8% to +1.3%. That establishes no resolved encode-and-submit overhead at this
noise floor. Order-controlled render differences likewise contract to within
2.3%. Raw and wgfx ultimately make the same WebGPU calls here, so these small
positive and negative values are treated as measurement variability, not
speedups or regressions. Queue completion remains millisecond scale.

The representative rendering workload varies from one to 1,000 indexed draws.
The representative compute workload updates a 16 KiB storage buffer and
dispatches 64 workgroups. Buffer-update experiments span 256 B to 256 KiB.

## Allocation and copying observations

Measured application-visible allocations:

- `createStorage` adds one 96-byte allocation for `wgfx::Uniform`; raw buffer
  creation adds none at this layer.
- Compute encoding is identical at three allocations/296 bytes per operation.
- Render encoding uses five allocations/368 bytes through wgfx versus three
  allocations/296 bytes raw. The two extra allocations correspond to temporary
  render-pass/attachment vector state. No stable timing penalty was resolved.
- Compute-pipeline creation uses nine allocations/880 bytes through wgfx versus
  four allocations/240 bytes raw.

Observed in source, but not claimed as profiler measurements:

- `createUniform` reserves `2560 * aligned_stride + size` bytes. A 64-byte
  uniform aligned to 256 bytes therefore allocates 655,424 bytes. This policy
  can dominate persistent memory and should become caller-sized or growable.
- Texture loading allocates a new CPU vector for every mip, copies level zero,
  and generates subsequent levels serially on the CPU before separate uploads.
- Vertex/index creation copies caller vectors into wrapper-owned vectors before
  uploading. Persistent lookup by full vector equality can add linear scans and
  comparisons when many resources exist.
- `RenderPass::prepare` constructs an attachment vector every pass.
- Storage updates do not make an extra wgfx copy; after bounds checking they
  forward the original pointer and size directly to `queue.writeBuffer`.

Texture upload is not assigned a raw-vs-wgfx number: the current public wgfx API
loads an encoded file and also performs CPU mip generation, while raw
`writeTexture` accepts already-decoded pixels. Those are different workloads.
A fair future benchmark requires a public `createTexture(width, height, pixels,
mipPolicy)` API shared by both paths.

## Synchronization and GPU execution

`wgfx::frame()` submits, presents, then calls both `device.poll(false)` and
blocking `device.poll(true)` on wgpu-native. The benchmark's closest equivalent
measured 1.233 ms median for that policy versus 1.194 ms for a single blocking
poll after the same dispatch. More importantly, either blocking poll serializes
every frame with GPU completion and prevents normal multi-frame overlap.

The optional `configureFrameExecution(true, collectMetrics, 3)` policy removes
the unconditional blocking poll while bounding outstanding submissions. Each
submission receives a queue-completion callback; when the bound is reached,
wgfx first retires completed submissions and waits only if all slots remain
occupied. `drainFrames()` provides an explicit synchronization point for
readback, reporting, and shutdown. WebGPU queue ordering protects persistent
buffers reused by later frames, and the bound keeps command resources and
callbacks from accumulating without limit.

In a Release wDoom demo-1 A/B (five alternating reversed-order runs per mode),
median throughput increased from 416.85 to 786.62 FPS (+88.7%) and median
run-level mean CPU frame time fell from 2.399 to 1.271 ms (-47.0%). Median GPU
queue-completion latency rose from 1.196 to 1.329 ms, as expected with work in
flight, and maximum observed peak RSS rose from 144.7 to 152.1 MiB. One
every-frame validation replay compared 109,440,000 indexed pixels with zero
mismatches. The unconditional wait was therefore a framework-level bottleneck
for wDoom rather than wrapper-call overhead.

The subsequent complete four-demo corpus confirmed correctness across 8,738
frames and 559,232,000 pixels with zero mismatches, readback failures, or
pending callbacks. Validation-off GPU walls completed the corpus in 8.660
seconds (1,009.0 FPS), while a matched nonblocking CPU-wall run took 6.732
seconds (1,297.9 FPS). Removing the wait closes part of the renderer gap but
does not eliminate it: GPU walls remain 22.3% slower on this workload, leaving
command preparation, transfers, and wall execution as the next targets.

This is consistent with wDoom's separate end-to-end result: its WebGPU wall path
averaged 246.6 realtics versus 166.6 for CPU walls on the same machine. That
48.0% longer duration is **not wgfx wrapper overhead**. It includes wall-command
construction, buffer uploads, compute work, and forced completion. Removing the
wrapper would not remove those costs.

GPU timestamp queries were not available in the requested device feature set,
so no shader-only duration is claimed. An Instruments Time Profiler trace was
successfully recorded (`xctrace` 26.0, 2.835 seconds), but automated XML export
did not provide reliably symbolicated aggregate source rows. Metal System Trace
and Allocations remain reproducible manual follow-ups rather than fabricated
numbers.

```sh
xcrun xctrace record --template 'Time Profiler' \
  --output wgfx-time.trace --launch -- ./wgfx_benchmark results.csv
xcrun xctrace record --template 'Allocations' \
  --output wgfx-allocations.trace --launch -- ./wgfx_benchmark results.csv
xcrun xctrace record --template 'Metal System Trace' \
  --output wgfx-metal.trace --launch -- ./wgfx_benchmark results.csv
```

## Bottlenecks and recommendations

1. **Stop blocking every frame.** Make completion waiting explicit and use it
   only for readback, shutdown, tests, or a bounded frames-in-flight policy.
   Expected impact: highest for wDoom; directly addresses lost CPU/GPU overlap.
2. **Benchmark and reduce upload volume.** wDoom's commands, snapshot, offsets,
   mask, and framebuffer updates matter far more than nanosecond wrapper calls.
   Prefer persistent buffers, dirty ranges, and GPU-side timestamps.
3. **Replace the fixed 2,560-entry uniform allocation policy.** Allocate the
   requested capacity or a documented growable ring buffer. Expected memory
   impact: very high for small uniforms.
4. **Make ownership RAII-complete.** Pipeline layouts created during pipeline
   initialization are not explicitly released, `IndexBuffer` has no matching
   destructor, and global resources have mixed ownership. Validate with the
   Allocations and Leaks templates before optimizing allocator microseconds.
5. **Remove unconditional logging from hot/setup APIs.** Route it through a
   compile-time or runtime diagnostic callback.
6. **Reuse render attachment storage.** A small fixed array or retained vector
   removes the two measured per-pass allocations. Expected CPU impact: small.
7. **Add a decoded-pixel texture API and optional GPU mip generation.** This
   enables a fair upload benchmark and avoids mandatory serial CPU mip work.

## Reproduction and interpretation

Build and run commands are in the repository README. CSV fields are
self-describing. Compare medians first, p95 for tail behavior, and standard
deviation before interpreting small deltas. Negative wrapper overhead means the
test did not resolve a cost; it does not mean wgfx accelerates WebGPU.
