#pragma once

#include <context.h>

using namespace wgpu;
namespace wgfx
{

	void init(Surface surface);

	void initSurface();

	Surface getSurface(SDL_Window* w);

	void frame();

	void initDepth();
	
}