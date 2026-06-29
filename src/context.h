/*
 * Copyright (c) 2026 Ethan Herman (e10b). All rights reserved.
 *
 * This source code is part of the wgfx graphics library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * For commercial licensing inquiries, proprietary integrations, or alternate
 * use-cases that fall outside the scope of the AGPLv3, please contact:
 * ethan34787@gmail.com
 */

#pragma once

#include <webgpu/webgpu.hpp>

#ifdef WGFX_ENABLE_SDL
#include <sdl2webgpu.h>
#include <SDL3/SDL.h>
#endif

#include <cstdint>
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

#ifdef WGFX_ENABLE_SDL
    inline SDL_Window* window = nullptr; // Initialize to nullptr
#endif

    inline BufferDescriptor bufferDesc;
    inline TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
    //inline TextureView depthTextureView = nullptr; // Initialize to nullptr

    inline RenderPassEncoder renderPass = nullptr;


    inline bool reset = false;
    inline uint64_t frameIndex = 0;

    inline int samples = 1;
    inline bool multiSample = false;

    inline int width = 0;
    inline int height = 0;

    inline wgpu::VertexFormat vec1f = VertexFormat::Float32;
    inline wgpu::VertexFormat vec2f = VertexFormat::Float32x2;
    inline wgpu::VertexFormat vec3f = VertexFormat::Float32x3;
    inline wgpu::VertexFormat vec4f = VertexFormat::Float32x4;
    inline wgpu::VertexFormat u32 = VertexFormat::Uint32;
}
