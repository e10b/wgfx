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

	void draw();

	bool close = false;
	SDL_Window* window;
};