// SPDX-License-Identifier: MIT

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "wgfx.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Clock = std::chrono::steady_clock;
std::atomic<uint64_t> gAllocations{0};
std::atomic<uint64_t> gAllocatedBytes{0};
std::atomic<bool> gTrackAllocations{false};
std::atomic<uint64_t> gDeviceErrors{0};
SDL_Window* gBenchmarkWindow = nullptr;

struct AllocationScope
{
    uint64_t startCount = gAllocations.load();
    uint64_t startBytes = gAllocatedBytes.load();
    AllocationScope() { gTrackAllocations = true; }
    ~AllocationScope() { gTrackAllocations = false; }
    uint64_t count() const { return gAllocations.load() - startCount; }
    uint64_t bytes() const { return gAllocatedBytes.load() - startBytes; }
};

struct Result
{
    std::string benchmark;
    std::string implementation;
    uint64_t workload = 0;
    uint64_t batch = 0;
    uint64_t samples = 0;
    double meanNs = 0;
    double medianNs = 0;
    double p95Ns = 0;
    double stddevNs = 0;
    double allocations = 0;
    double allocatedBytes = 0;
    uint64_t uploadBytes = 0;
};

template <typename Function>
Result Measure(const std::string& benchmark, const std::string& implementation,
               uint64_t workload, uint64_t batch, uint64_t samples,
               uint64_t uploadBytes, bool drainAfterSample, Function function)
{
    for (int warmup = 0; warmup < 10; ++warmup)
        for (uint64_t item = 0; item < batch; ++item)
            function();

    if (drainAfterSample)
        wgfx::device.poll(true);
    std::vector<double> timings;
    timings.reserve(samples);
    uint64_t allocationCount = 0;
    uint64_t allocationBytes = 0;
    for (uint64_t sample = 0; sample < samples; ++sample)
    {
        {
            AllocationScope allocations;
            const auto start = Clock::now();
            for (uint64_t item = 0; item < batch; ++item)
                function();
            const auto elapsed = std::chrono::duration<double, std::nano>(
                Clock::now() - start).count();
            allocationCount += allocations.count();
            allocationBytes += allocations.bytes();
            timings.push_back(elapsed / static_cast<double>(batch));
        }
        // Keep asynchronous queue depth equivalent without charging completion
        // latency to CPU submission/queue-write measurements.
        if (drainAfterSample)
            wgfx::device.poll(true);
    }
    std::sort(timings.begin(), timings.end());
    const double mean = std::accumulate(timings.begin(), timings.end(), 0.0) /
                        timings.size();
    double variance = 0;
    for (double timing : timings)
        variance += (timing - mean) * (timing - mean);
    variance /= timings.size();
    return {benchmark, implementation, workload, batch, samples, mean,
            timings[timings.size() / 2],
            timings[static_cast<size_t>(std::ceil(timings.size() * 0.95)) - 1],
            std::sqrt(variance),
            static_cast<double>(allocationCount) / (samples * batch),
            static_cast<double>(allocationBytes) / (samples * batch), uploadBytes};
}

template <typename RawFunction, typename WgfxFunction>
std::pair<Result, Result> MeasureAlternating(
    const std::string& benchmark, uint64_t workload, uint64_t batch,
    uint64_t samples, uint64_t uploadBytes, bool drainAfterSample,
    RawFunction rawFunction, WgfxFunction wgfxFunction)
{
    for (int warmup = 0; warmup < 10; ++warmup)
    {
        for (uint64_t item = 0; item < batch; ++item)
            rawFunction();
        for (uint64_t item = 0; item < batch; ++item)
            wgfxFunction();
        if (drainAfterSample)
            wgfx::device.poll(true);
    }

    std::vector<double> rawTimings;
    std::vector<double> wgfxTimings;
    rawTimings.reserve(samples);
    wgfxTimings.reserve(samples);
    uint64_t rawAllocationCount = 0;
    uint64_t rawAllocationBytes = 0;
    uint64_t wgfxAllocationCount = 0;
    uint64_t wgfxAllocationBytes = 0;

    auto measureOne = [&](auto& function, std::vector<double>& timings,
                          uint64_t& allocationCount,
                          uint64_t& allocationBytes)
    {
        {
            AllocationScope allocations;
            const auto start = Clock::now();
            for (uint64_t item = 0; item < batch; ++item)
                function();
            timings.push_back(std::chrono::duration<double, std::nano>(
                Clock::now() - start).count() / static_cast<double>(batch));
            allocationCount += allocations.count();
            allocationBytes += allocations.bytes();
        }
        if (drainAfterSample)
            wgfx::device.poll(true);
    };

    for (uint64_t sample = 0; sample < samples; ++sample)
    {
        // Reverse the order every sample so thermal, queue, and cache drift do
        // not consistently favor either implementation.
        if ((sample & 1u) == 0)
        {
            measureOne(rawFunction, rawTimings, rawAllocationCount,
                       rawAllocationBytes);
            measureOne(wgfxFunction, wgfxTimings, wgfxAllocationCount,
                       wgfxAllocationBytes);
        }
        else
        {
            measureOne(wgfxFunction, wgfxTimings, wgfxAllocationCount,
                       wgfxAllocationBytes);
            measureOne(rawFunction, rawTimings, rawAllocationCount,
                       rawAllocationBytes);
        }
    }

    auto summarize = [&](const std::string& implementation,
                         std::vector<double>& timings, uint64_t allocationCount,
                         uint64_t allocationBytes)
    {
        std::sort(timings.begin(), timings.end());
        const double mean = std::accumulate(timings.begin(), timings.end(), 0.0) /
                            timings.size();
        double variance = 0;
        for (double timing : timings)
            variance += (timing - mean) * (timing - mean);
        variance /= timings.size();
        return Result{benchmark, implementation, workload, batch, samples, mean,
            timings[timings.size() / 2],
            timings[static_cast<size_t>(std::ceil(timings.size() * 0.95)) - 1],
            std::sqrt(variance),
            static_cast<double>(allocationCount) / (samples * batch),
            static_cast<double>(allocationBytes) / (samples * batch), uploadBytes};
    };

    return {summarize("raw", rawTimings, rawAllocationCount, rawAllocationBytes),
            summarize("wgfx", wgfxTimings, wgfxAllocationCount,
                      wgfxAllocationBytes)};
}

void AppendPair(std::vector<Result>& results, std::pair<Result, Result> pair)
{
    results.push_back(std::move(pair.first));
    results.push_back(std::move(pair.second));
}

constexpr const char* kComputeShader = R"WGSL(
@group(0) @binding(0) var<storage, read_write> values: array<u32>;
@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x < arrayLength(&values)) { values[id.x] = values[id.x] + 1u; }
}
)WGSL";

constexpr const char* kRenderShader = R"WGSL(
struct VertexOut { @builtin(position) position: vec4<f32> };
@vertex fn vs_main(@location(0) position: vec2<f32>) -> VertexOut {
    var result: VertexOut;
    result.position = vec4<f32>(position, 0.0, 1.0);
    return result;
}
@fragment fn fs_main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.25, 0.5, 0.75, 1.0);
}
)WGSL";

wgpu::ShaderModule CreateShader(const char* source)
{
    wgpu::ShaderModuleWGSLDescriptor wgsl{};
    wgsl.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    wgsl.code = source;
    wgpu::ShaderModuleDescriptor descriptor{};
    descriptor.nextInChain = &wgsl.chain;
    descriptor.hintCount = 0;
    descriptor.hints = nullptr;
    return wgfx::device.createShaderModule(descriptor);
}

struct RawCompute
{
    wgpu::BindGroupLayout layout = nullptr;
    wgpu::BindGroup bindGroup = nullptr;
    wgpu::PipelineLayout pipelineLayout = nullptr;
    wgpu::ComputePipeline pipeline = nullptr;
};

RawCompute CreateRawCompute(wgpu::Buffer buffer, uint64_t size)
{
    RawCompute result;
    wgpu::BindGroupLayoutEntry layoutEntry{};
    layoutEntry.binding = 0;
    layoutEntry.visibility = wgpu::ShaderStage::Compute;
    layoutEntry.buffer.type = wgpu::BufferBindingType::Storage;
    layoutEntry.buffer.minBindingSize = size;
    wgpu::BindGroupLayoutDescriptor layoutDescriptor{};
    layoutDescriptor.entryCount = 1;
    layoutDescriptor.entries = &layoutEntry;
    result.layout = wgfx::device.createBindGroupLayout(layoutDescriptor);

    wgpu::BindGroupEntry entry{};
    entry.binding = 0;
    entry.buffer = buffer;
    entry.size = size;
    wgpu::BindGroupDescriptor bindDescriptor{};
    bindDescriptor.layout = result.layout;
    bindDescriptor.entryCount = 1;
    bindDescriptor.entries = &entry;
    result.bindGroup = wgfx::device.createBindGroup(bindDescriptor);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{};
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts =
        reinterpret_cast<WGPUBindGroupLayout*>(&result.layout);
    result.pipelineLayout =
        wgfx::device.createPipelineLayout(pipelineLayoutDescriptor);
    wgpu::ShaderModule shader = CreateShader(kComputeShader);
    wgpu::ComputePipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.layout = result.pipelineLayout;
    pipelineDescriptor.compute.module = shader;
    pipelineDescriptor.compute.entryPoint = "main";
    result.pipeline = wgfx::device.createComputePipeline(pipelineDescriptor);
    shader.release();
    return result;
}

void ReleaseRawCompute(RawCompute& compute)
{
    compute.pipeline.release();
    compute.pipelineLayout.release();
    compute.bindGroup.release();
    compute.layout.release();
}

wgpu::CommandBuffer EncodeRaw(const RawCompute& compute, uint32_t groups)
{
    wgpu::CommandEncoder encoder = wgfx::device.createCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.beginComputePass();
    pass.setPipeline(compute.pipeline);
    pass.setBindGroup(0, compute.bindGroup, 0, nullptr);
    pass.dispatchWorkgroups(groups, 1, 1);
    pass.end();
    pass.release();
    wgpu::CommandBuffer command = encoder.finish();
    encoder.release();
    return command;
}

wgpu::CommandBuffer EncodeWgfx(wgfx::Compute* compute, uint32_t groups)
{
    wgfx::encoder = wgfx::device.createCommandEncoder();
    wgfx::ComputePass pass;
    pass.prepare();
    pass.draw(compute, groups);
    pass.end();
    wgpu::CommandBuffer command = wgfx::encoder.finish();
    wgfx::encoder.release();
    wgfx::encoder = nullptr;
    return command;
}

wgpu::CommandBuffer EncodeRawRender(wgpu::TextureView target,
                                    wgpu::RenderPipeline pipeline,
                                    wgpu::BindGroup bindGroup,
                                    wgpu::Buffer vertices,
                                    wgpu::Buffer indices,
                                    uint32_t drawCalls)
{
    wgpu::CommandEncoder encoder = wgfx::device.createCommandEncoder();
    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = target;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassDescriptor descriptor{};
    descriptor.colorAttachmentCount = 1;
    descriptor.colorAttachments = &attachment;
    wgpu::RenderPassEncoder pass = encoder.beginRenderPass(descriptor);
    for (uint32_t draw = 0; draw < drawCalls; ++draw)
    {
        pass.setBindGroup(0, bindGroup, 0, nullptr);
        pass.setPipeline(pipeline);
        pass.setVertexBuffer(0, vertices, 0, vertices.getSize());
        pass.setIndexBuffer(indices, wgpu::IndexFormat::Uint16, 0, indices.getSize());
        pass.drawIndexed(3, 1, 0, 0, 0);
    }
    pass.end(); pass.release();
    wgpu::CommandBuffer command = encoder.finish();
    encoder.release();
    return command;
}

wgpu::CommandBuffer EncodeWgfxRender(wgfx::ColorTexture* target,
                                     wgfx::Pipeline* pipeline,
                                     uint32_t drawCalls)
{
    wgfx::encoder = wgfx::device.createCommandEncoder();
    wgfx::RenderPass pass;
    pass.addTarget(target);
    pass.prepare();
    for (uint32_t draw = 0; draw < drawCalls; ++draw)
        pass.draw(pipeline);
    pass.end();
    wgpu::CommandBuffer command = wgfx::encoder.finish();
    wgfx::encoder.release();
    wgfx::encoder = nullptr;
    return command;
}

void InitializeDevice()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    gBenchmarkWindow = SDL_CreateWindow(
        "wgfx benchmark", 64, 64, SDL_WINDOW_HIDDEN);
    if (!gBenchmarkWindow)
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") +
                                 SDL_GetError());
    wgpu::Surface benchmarkSurface = wgfx::getSurface(gBenchmarkWindow);
    if (!benchmarkSurface)
        throw std::runtime_error("Could not create benchmark WebGPU surface");
    wgfx::init(benchmarkSurface);
    wgfx::uncapturedErrorCallbackHandle = wgfx::device.setUncapturedErrorCallback(
        [](wgpu::ErrorType type, const char* message)
        {
            ++gDeviceErrors;
            std::cerr << "WebGPU benchmark device error " << type << ": "
                      << (message ? message : "") << '\n';
        });
}

void WriteCsv(const std::string& path, const std::vector<Result>& results)
{
    std::ofstream output(path);
    output << "benchmark,implementation,workload_units,batch,samples,mean_ns,median_ns,p95_ns,stddev_ns,allocations_per_op,allocated_bytes_per_op,upload_bytes_per_op\n";
    output << std::fixed << std::setprecision(3);
    for (const Result& result : results)
        output << result.benchmark << ',' << result.implementation << ','
               << result.workload << ',' << result.batch << ',' << result.samples
               << ',' << result.meanNs << ',' << result.medianNs << ','
               << result.p95Ns << ',' << result.stddevNs << ','
               << result.allocations << ',' << result.allocatedBytes << ','
               << result.uploadBytes << '\n';
}
} // namespace

void* operator new(std::size_t size)
{
    if (gTrackAllocations)
    {
        ++gAllocations;
        gAllocatedBytes += size;
    }
    if (void* memory = std::malloc(size))
        return memory;
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

int main(int argc, char** argv)
{
    try
    {
        const std::string outputPath = argc > 1 ? argv[1] : "wgfx-benchmark.csv";
        InitializeDevice();
        std::vector<Result> results;
        const std::vector<uint64_t> sizes = {256, 4096, 262144};

        for (uint64_t size : sizes)
        {
            std::vector<uint8_t> data(size, 7);
            results.push_back(Measure("storage_create_destroy", "raw", size, 1, 80, 0, false, [&] {
                wgpu::BufferDescriptor descriptor{};
                descriptor.size = size;
                descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage |
                                   wgpu::BufferUsage::CopySrc;
                wgpu::Buffer buffer = wgfx::device.createBuffer(descriptor);
                buffer.destroy(); buffer.release();
            }));
            results.push_back(Measure("storage_create_destroy", "wgfx", size, 1, 80, 0, false, [&] {
                delete wgfx::createStorage(0, size, nullptr, false);
            }));

            wgpu::BufferDescriptor descriptor{};
            descriptor.size = size;
            descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage |
                               wgpu::BufferUsage::CopySrc;
            wgpu::Buffer rawBuffer = wgfx::device.createBuffer(descriptor);
            wgfx::Uniform* storage = wgfx::createStorage(0, size, nullptr, false);
            wgfx::Compute updateWrapper;
            results.push_back(Measure("storage_update", "raw", size, 32, 100, size, true, [&] {
                wgfx::queue.writeBuffer(rawBuffer, 0, data.data(), size);
            }));
            results.push_back(Measure("storage_update", "wgfx", size, 32, 100, size, true, [&] {
                updateWrapper.updateStorageBuffer(storage, data.data(), size, 0);
            }));
            rawBuffer.destroy(); rawBuffer.release();
            delete storage;
        }

        constexpr uint64_t computeBytes = 4096 * sizeof(uint32_t);
        wgpu::BufferDescriptor bufferDescriptor{};
        bufferDescriptor.size = computeBytes;
        bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage |
                                 wgpu::BufferUsage::CopySrc;
        wgpu::Buffer rawBuffer = wgfx::device.createBuffer(bufferDescriptor);
        RawCompute rawCompute = CreateRawCompute(rawBuffer, computeBytes);
        wgfx::Uniform* storage = wgfx::createStorage(0, computeBytes, nullptr, false);
        wgfx::Compute* wrappedCompute = wgfx::loadCompute(kComputeShader);
        wrappedCompute->setStorage(storage);
        wrappedCompute->init();

        results.push_back(Measure("compute_encode_dispatch", "raw", computeBytes, 16, 120, 0, false, [&] {
            wgpu::CommandBuffer command = EncodeRaw(rawCompute, 64);
            command.release();
        }));
        results.push_back(Measure("compute_encode_dispatch", "wgfx", computeBytes, 16, 120, 0, false, [&] {
            wgpu::CommandBuffer command = EncodeWgfx(wrappedCompute, 64);
            command.release();
        }));

        AppendPair(results, MeasureAlternating(
            "compute_submit", computeBytes, 8, 160, 0, true,
            [&] {
                wgpu::CommandBuffer command = EncodeRaw(rawCompute, 64);
                wgfx::queue.submit(1, &command); command.release();
            },
            [&] {
                wgpu::CommandBuffer command = EncodeWgfx(wrappedCompute, 64);
                wgfx::queue.submit(1, &command); command.release();
            }));

        results.push_back(Measure("compute_submit_and_wait", "raw", computeBytes, 1, 60, 0, false, [&] {
            wgpu::CommandBuffer command = EncodeRaw(rawCompute, 64);
            wgfx::queue.submit(1, &command); command.release();
            wgfx::device.poll(true);
        }));
        results.push_back(Measure("compute_submit_and_wait", "wgfx", computeBytes, 1, 60, 0, false, [&] {
            wgpu::CommandBuffer command = EncodeWgfx(wrappedCompute, 64);
            wgfx::queue.submit(1, &command); command.release();
            wgfx::device.poll(false); wgfx::device.poll(true);
        }));

        auto recreateRawBindings = [&] {
            rawCompute.bindGroup.release();
            rawCompute.layout.release();
            wgpu::BindGroupLayoutEntry layoutEntry{};
            layoutEntry.binding = 0;
            layoutEntry.visibility = wgpu::ShaderStage::Compute;
            layoutEntry.buffer.type = wgpu::BufferBindingType::Storage;
            layoutEntry.buffer.minBindingSize = computeBytes;
            wgpu::BindGroupLayoutDescriptor layoutDescriptor{};
            layoutDescriptor.entryCount = 1;
            layoutDescriptor.entries = &layoutEntry;
            rawCompute.layout = wgfx::device.createBindGroupLayout(layoutDescriptor);
            wgpu::BindGroupEntry entry{};
            entry.binding = 0;
            entry.buffer = rawBuffer;
            entry.size = computeBytes;
            wgpu::BindGroupDescriptor bindDescriptor{};
            bindDescriptor.layout = rawCompute.layout;
            bindDescriptor.entryCount = 1;
            bindDescriptor.entries = &entry;
            rawCompute.bindGroup = wgfx::device.createBindGroup(bindDescriptor);
        };
        results.push_back(Measure("bind_group_recreate", "raw", 1, 1, 100, 0, false,
            recreateRawBindings));
        results.push_back(Measure("bind_group_recreate", "wgfx", 1, 1, 100, 0, false, [&] {
            wrappedCompute->uniforms.touch();
        }));

        results.push_back(Measure("compute_pipeline_create", "raw", computeBytes, 1, 12, 0, false, [&] {
            RawCompute temporary = CreateRawCompute(rawBuffer, computeBytes);
            ReleaseRawCompute(temporary);
        }));
        results.push_back(Measure("compute_pipeline_create", "wgfx", computeBytes, 1, 12, 0, false, [&] {
            wgfx::Compute* temporary = wgfx::loadCompute(kComputeShader);
            temporary->setStorage(storage);
            temporary->init();
            temporary->pipeline.release();
            delete temporary;
        }));

        // Representative offscreen rendering uses the exact same WebGPU
        // resources for both encoders. Only command construction differs.
        wgpu::TextureDescriptor textureDescriptor{};
        textureDescriptor.size = {256, 256, 1};
        textureDescriptor.dimension = wgpu::TextureDimension::_2D;
        textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.sampleCount = 1;
        wgpu::Texture renderTexture = wgfx::device.createTexture(textureDescriptor);
        wgpu::TextureView renderView = renderTexture.createView();

        const std::vector<float> vertexData = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};
        const std::vector<uint16_t> indexData = {0, 1, 2};
        wgfx::VertexBuffer* vertexBuffer = wgfx::createVertexBuffer(vertexData);
        vertexBuffer->setAttribute(0, wgpu::VertexFormat::Float32x2, 0);
        wgfx::IndexBuffer* indexBuffer = wgfx::createIndexBuffer(indexData);

        wgpu::BindGroupLayoutDescriptor emptyLayoutDescriptor{};
        wgpu::BindGroupLayout emptyLayout =
            wgfx::device.createBindGroupLayout(emptyLayoutDescriptor);
        wgpu::BindGroupDescriptor emptyBindDescriptor{};
        emptyBindDescriptor.layout = emptyLayout;
        wgpu::BindGroup emptyBindGroup =
            wgfx::device.createBindGroup(emptyBindDescriptor);
        wgpu::PipelineLayoutDescriptor renderLayoutDescriptor{};
        renderLayoutDescriptor.bindGroupLayoutCount = 1;
        renderLayoutDescriptor.bindGroupLayouts =
            reinterpret_cast<WGPUBindGroupLayout*>(&emptyLayout);
        wgpu::PipelineLayout renderLayout =
            wgfx::device.createPipelineLayout(renderLayoutDescriptor);
        wgpu::ShaderModule renderShader = CreateShader(kRenderShader);
        wgpu::VertexAttribute vertexAttribute{};
        vertexAttribute.shaderLocation = 0;
        vertexAttribute.format = wgpu::VertexFormat::Float32x2;
        wgpu::VertexBufferLayout vertexLayout{};
        vertexLayout.arrayStride = 2 * sizeof(float);
        vertexLayout.attributeCount = 1;
        vertexLayout.attributes = &vertexAttribute;
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::FragmentState fragment{};
        fragment.module = renderShader;
        fragment.entryPoint = "fs_main";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        wgpu::RenderPipelineDescriptor renderPipelineDescriptor{};
        renderPipelineDescriptor.layout = renderLayout;
        renderPipelineDescriptor.vertex.module = renderShader;
        renderPipelineDescriptor.vertex.entryPoint = "vs_main";
        renderPipelineDescriptor.vertex.bufferCount = 1;
        renderPipelineDescriptor.vertex.buffers = &vertexLayout;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        renderPipelineDescriptor.primitive.frontFace = wgpu::FrontFace::CCW;
        renderPipelineDescriptor.primitive.cullMode = wgpu::CullMode::None;
        renderPipelineDescriptor.fragment = &fragment;
        renderPipelineDescriptor.multisample.count = 1;
        renderPipelineDescriptor.multisample.mask = ~0u;
        wgpu::RenderPipeline renderPipeline =
            wgfx::device.createRenderPipeline(renderPipelineDescriptor);
        renderShader.release();

        wgfx::ColorTexture wrappedTarget;
        wrappedTarget.colorView = renderView;
        wgfx::Pipeline wrappedRenderPipeline;
        wrappedRenderPipeline.pipeline = renderPipeline;
        wrappedRenderPipeline.vbos.current = vertexBuffer;
        wrappedRenderPipeline.ibos.current = indexBuffer;
        wrappedRenderPipeline.uniforms.bindGroupLayout = emptyLayout;
        wrappedRenderPipeline.uniforms.bindGroup = emptyBindGroup;
        for (uint32_t drawCalls : {1u, 100u, 1000u})
        {
            const uint64_t samples = drawCalls == 1000 ? 40 : 100;
            AppendPair(results, MeasureAlternating(
                "render_encode", drawCalls, 1, samples, 0, false,
                [&] {
                    wgpu::CommandBuffer command = EncodeRawRender(
                        renderView, renderPipeline, emptyBindGroup,
                        vertexBuffer->buffer, indexBuffer->buffer, drawCalls);
                    command.release();
                },
                [&] {
                    wgpu::CommandBuffer command = EncodeWgfxRender(
                        &wrappedTarget, &wrappedRenderPipeline, drawCalls);
                    command.release();
                }));
        }

        wgfx::device.poll(true);
        if (gDeviceErrors != 0)
            throw std::runtime_error("WebGPU validation failed; refusing to write benchmark results");
        WriteCsv(outputPath, results);
        std::cout << "Wrote " << results.size() << " benchmark rows to " << outputPath << "\n";
        ReleaseRawCompute(rawCompute);
        rawBuffer.destroy(); rawBuffer.release();
        delete wrappedCompute;
        delete storage;
        // Detach shared handles before wrapper destructors run; release each
        // WebGPU object exactly once below.
        wrappedTarget.colorView = nullptr;
        wrappedRenderPipeline.pipeline = nullptr;
        wrappedRenderPipeline.uniforms.bindGroup = nullptr;
        wrappedRenderPipeline.uniforms.bindGroupLayout = nullptr;
        delete vertexBuffer;
        indexBuffer->buffer.destroy(); indexBuffer->buffer.release();
        delete indexBuffer;
        renderPipeline.release(); renderLayout.release();
        emptyBindGroup.release(); emptyLayout.release();
        renderView.release(); renderTexture.release();
        wgfx::device.poll(true);
        if (gBenchmarkWindow)
            SDL_DestroyWindow(gBenchmarkWindow);
        SDL_Quit();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "wgfx benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
