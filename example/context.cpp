#include "context.h"

Context::Context()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "Couldn't init SDL!\n"; }
	window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);
	wgfx::init(wgfx::getSurface(window));

}

void Context::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_WINDOW_RESIZED:
		{
			wgfx::initSurface();
			wgfx::initDepth();

			int width, height;
			SDL_GetWindowSize(window, &width, &height);

			//proj = glm::perspective(glm::radians(50.0f), float(width) / float(height), 0.1f, 100.0f);
			//pipeline.updateUniform(projUniform, glm::value_ptr(proj));
		}
		break;

		case SDL_EVENT_QUIT:
			close = true;
			break;

		case SDL_EVENT_KEY_DOWN:
			if (event.key.key == SDLK_ESCAPE) {
				close = true; // Close the application if Escape is pressed
			}
			break;

		case SDL_EVENT_WINDOW_EXPOSED:
			wgfx::initSurface();
			break;
		}
	}
}

void Context::draw()
{
	wgfx::frame();
}

