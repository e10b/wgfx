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

#include <context.h>

using namespace wgpu;
namespace wgfx
{

	void init();

	void init(Surface surface);

#ifdef WGFX_ENABLE_SDL
	void initSurface();

	Surface getSurface(SDL_Window* w);
#endif

	void frame();

	void initDepth();
	
}
