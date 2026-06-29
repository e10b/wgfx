#include "context.h"

Context::Context()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "Couldn't init SDL!\n"; }
	window = SDL_CreateWindow("wgfx examples", 1280, 720, SDL_WINDOW_RESIZABLE);
	wgfx::init(wgfx::getSurface(window));
}

void Context::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (eventCallback) {
			eventCallback(event);
		}
		switch (event.type)
		{
		case SDL_EVENT_WINDOW_RESIZED:
		{
			wgfx::initSurface();
			int width, height;
			SDL_GetWindowSize(window, &width, &height);
		}
		break;

		case SDL_EVENT_QUIT:
			close = true;
			break;

		case SDL_EVENT_KEY_DOWN:
			if (event.key.key == SDLK_ESCAPE) {
				close = true;
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
