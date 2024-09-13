#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<float> pointData = {
		-1, -1,  1,	
		 1, -1,  1,	
		-1,  1,  1,	
		 1,  1,  1,
		-1, -1, -1,	
		 1, -1, -1,	
		-1,  1, -1,	
		 1,  1, -1,	
};

std::vector<uint16_t> indexData = {
		//Top
		2, 6, 7,
		2, 3, 7,

		//Bottom
		0, 4, 5,
		0, 1, 5,

		//Left
		0, 2, 6,
		0, 4, 6,

		//Right
		1, 3, 7,
		1, 5, 7,

		//Front
		0, 2, 3,
		0, 1, 3,

		//Back
		4, 6, 7,
		4, 5, 7
};


int main(int _argc, char** _argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);

	wgfx::init(wgfx::getSurface(window), 1280, 720);

	wgfx::Program program = wgfx::loadProgram(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));
	
	wgfx::IndexBuffer ibo(indexData);
	wgfx::VertexBuffer vbo(pointData, 3);
		vbo.setAttribute(0, wgfx::vec3f, 0);


	glm::mat4 proj = glm::perspective(glm::radians(50.0f), float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(80.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

	wgfx::Uniform viewUniform(0, sizeof(glm::mat4), glm::value_ptr(view));
	wgfx::Uniform projUniform(1, sizeof(glm::mat4), glm::value_ptr(proj));

	program.setUniform(viewUniform);
	program.setUniform(projUniform);

	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	bool shouldClose = false;
	while (!shouldClose)
	{
		SDL_Event event;
		float t = SDL_GetTicks() / 1000.0f;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
			{
				int newWidth = event.window.data1;
				int newHeight = event.window.data2;
				float aspectRatio = (float)newWidth / (float)newHeight;

				wgfx::initSurface();
				wgfx::initDepth(newWidth, newHeight);


				proj = glm::perspective(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);
				program.updateUniform(projUniform, glm::value_ptr(proj));
			}
			break;

			case SDL_EVENT_QUIT:
				shouldClose = true;
				break;

			case SDL_EVENT_WINDOW_EXPOSED:
				wgfx::initSurface();

			default:
				break;
			}
		}


		view = glm::rotate(view, 0.02f, glm::vec3(0.4f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

		program.updateUniform(viewUniform, glm::value_ptr(view));

		wgfx::submit(program);

	}

}