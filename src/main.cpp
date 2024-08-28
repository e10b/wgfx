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
	



	// right here we go

	// wgfx::Init(wgfx::getSurface(window), 1920, 1080);

	// wgfx::VertexBuffer vertex = wgfx::createVertexBuffer(data*);
	//vertex.setAttribute(0, vec3);

	// wgfx::Program program = wgfx::loadProgram(shaderSource);

				/*
					query how many vertex buffers can you have well as many as you like then how correlate to program ?
					gosh golly

					bleedin hart just throw it?

					program.setVertexBuffer(vertex); well umm, no, maybe? i am thinking copy copy copy copy copy;
				*/


	
	//for (;;)
	//{

		//bgfx::setVertexBuffer( << this is where the struggle is
		// i think it would make more sense
		// order of  things
		// wgfx::setVertexBuffer(0, buffer); or else wgfx::setVertexBuffer(buffer);
		// wgfx::setIndexBuffer( debroglie);
		
		//wgfx::submit(view, program_); not gonna lie, kinda like the 0, structure, most of the time you need like 2 or 3 views max.
	//bugger, i start to copy.. shamelessly

	//}

	//wgfx::frame(); << send pipeline etc. 





				// indead why create a well, he calls it program handle and vertexbufferhandle but its just an object really





	// whats important right now is the next milestone
	// right now we have a colored window, next i want to draw a triangle
	// in what way? well, i want to create a buffer which contains vertexdata
	// and a single attribute, pos.
	// pos of the vertices that is.
	// thats all, of course that implies sending with a shader, i want to send the shader as a const char* for now.
	// creating a shader implies the creation of a pipeline.

	// when we call frame it should say HEY LOOK WAS THERE A SHADER? oh oh oh yes there was let use prep and use it!

















	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL! Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	int windowFlags = SDL_WINDOW_RESIZABLE;//SDL_WINDOW_RESIZABLE;
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, windowFlags);

	wgfx::init(wgfx::getSurface(window), 1280, 720);

	/*wgfx::Program program = wgfx::loadProgram(sr);

	wgfx::VertexBuffer vbo;
	vbo.setAttribute(0, 0);
	program.setVertexBuffer(vbo);*/
	//vbo.setAttribute(0, vec3); // le type




	bool shouldClose = false;
	while (!shouldClose)
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




		// hmm pseudocode

		//wgfx::
	}

}