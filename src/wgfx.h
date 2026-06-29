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

#ifdef WGPU_IMPLEMENTATION
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#ifdef WGFX_ENABLE_SDL
#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL3/SDL.h>
#endif

#endif

#include <surface.h>
#include <pipeline.h>
#include <renderpass.h>
#include <texture.h>
