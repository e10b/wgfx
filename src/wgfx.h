#pragma once

#ifdef WGPU_IMPLEMENTATION
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL3/SDL.h>

#endif

#include <surface.h>
#include <pipeline.h>
#include <renderpass.h>
#include <texture.h>
