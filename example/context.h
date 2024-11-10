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
	void fps(float deltaTime)
	{
		// Calculate FPS as the reciprocal of deltaTime
		int fps = static_cast<int>(1.0f / deltaTime);

		// Update the window title with the FPS
		std::string title = "Learn WebGPU - FPS: " + std::to_string(fps);
		SDL_SetWindowTitle(window, title.c_str());
	}

	void update();

	void draw();

	bool close = false;
	SDL_Window* window;
};