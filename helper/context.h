#pragma once

#include <wgfx.h>
#include <string>

class Context
{
public:
	static Context& Instance()
	{
		static Context instance;
		return instance;
	}

	Context();
	void fps(float averageFrameTime)
	{
		float fps = 1.0f / averageFrameTime;
		std::string title = "Learn WebGPU - FPS: " + std::to_string(static_cast<int>(fps + 0.5f));
		SDL_SetWindowTitle(window, title.c_str());
	}

	void update();
	void draw();

	bool close = false;
	SDL_Window* window;
};
