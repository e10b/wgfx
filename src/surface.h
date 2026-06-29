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
