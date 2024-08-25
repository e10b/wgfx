#include "wgfx.h"

const char* sr = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

int main(int _argc, char** _argv)
{
	/*
	Context& context = Context::getInstance();
	context.init();

	Shader shader(sr);

	while (context.isRunning())
	{
		//context.update();

		context.encode();

		shader.use();
		
		context.submit();


		//context.draw();

	}

	*/

	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL! Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	int windowFlags = SDL_WINDOW_RESIZABLE;//SDL_WINDOW_RESIZABLE;
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, windowFlags);

	wgfx::Init init;
	init.surface = wgfx::getSurface(window);
	init.width = 1280;
	init.height = 720;
	wgfx::init(init);

	bool shouldClose = false;
	while(!shouldClose)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				shouldClose = true;
				break;

			default:
				break;
			}
		}

		wgfx::loop();
	}

}