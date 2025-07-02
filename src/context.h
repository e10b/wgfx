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
    inline TextureView depthTextureView = nullptr; // Initialize to nullptr // why is this global? Feels wrong, feels like it oughn't be global, renderpass member? hmm
    inline RenderPassEncoder renderPass = nullptr;
    
        
    inline TextureView depthView = nullptr; // i am making another one so it can be more static << for the purpose of rendering depth visually

    inline bool reset = false;

    inline int samples = 1; // doesn't work for depth sampling rn
    inline bool multiSample = false;

    inline int width = 0;
    inline int height = 0;

    inline bool resetDepth = true;

    // Inline type definitions
    inline wgpu::VertexFormat vec2f = VertexFormat::Float32x2;
    inline wgpu::VertexFormat vec3f = VertexFormat::Float32x3;
}
