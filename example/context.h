#pragma once

#include <wgfx.h>

class Context
{
public:
	static Context& Instance()
	{
		static Context instance;
		return instance;
	}

	Context();

	void update();

	bool close = false;
	SDL_Window* window;
};