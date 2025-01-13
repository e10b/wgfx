#pragma once

#include <context.h>

using namespace wgpu;
namespace wgfx
{
	inline void clear()
	{
		reset = true;
	}
	void init(Surface surface);

	void initSurface();

	Surface getSurface(SDL_Window* w);

	void frame();

	void initDepth();
	
}