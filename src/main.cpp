#include "context.h"

#include "shader.h"

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
		//return 1;
	}

	int windowFlags = 0;//SDL_WINDOW_RESIZABLE;
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);

	wgfx::init(window);

	for (;;)
	{
		wgfx::loop();
	}

}