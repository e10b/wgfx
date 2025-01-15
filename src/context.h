#pragma once

#include <webgpu/webgpu.hpp>
#include <sdl2webgpu.h>
#include <SDL3/SDL.h>
#include <memory> // For std::unique_ptr

using namespace wgpu;
namespace wgfx
{
    // global variable definitions
    inline Device device = nullptr;
    inline Queue queue = nullptr;
    inline Surface surface = nullptr;
    inline Adapter adapter = nullptr;
    inline Instance instance = nullptr;
    inline CommandEncoder encoder = nullptr;
    inline Limits deviceLimits;
    inline std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;
    inline TextureFormat surfaceFormat = TextureFormat::Undefined;
    inline RenderPipeline pipeline;
    inline SDL_Window* window = nullptr; // Initialize to nullptr
    inline BufferDescriptor bufferDesc;
    inline TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
    inline TextureView depthTextureView = nullptr; // Initialize to nullptr

    inline bool reset = false;

    inline int samples = 4;
    inline bool multiSample = true;

    inline int width = 0;
    inline int height = 0;

    // Inline type definitions
    inline wgpu::VertexFormat vec2f = VertexFormat::Float32x2;
    inline wgpu::VertexFormat vec3f = VertexFormat::Float32x3;
}
