#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
/*
std::vector<float> pointData = {
	// x,   y,     r,   g,   b
	-0.5, -0.5, -0.3,   0.0, 1.0, 1.0, // Point #0
	+0.5, -0.5, -0.3,   0.0, 1.0, 0.0, // Point #1
	+0.5, +0.5, -0.3,   1.0, 0.0, 1.0, // Point #2
	-0.5, +0.5, -0.3,   1.0, 1.0, 0.0,  // Point #3

	0, 0, 0.5,			0.5, 0.5, 0.5
};
*/

std::vector<float> pointData = {
		-1, -1,  1,		0.0, 1.0, 1.0,	 //0
		 1, -1,  1,		1.0, 1.0, 0.0,	 //1
		-1,  1,  1,		0.0, 0.0, 1.0,	 //2
		 1,  1,  1,		1.0, 1.0, 0.0,	 //3
		-1, -1, -1,		1.0, 0.0, 1.0,	 //4
		 1, -1, -1,		0.0, 1.0, 0.0,	 //5
		-1,  1, -1,		1.0, 0.0, 1.0,	 //6
		 1,  1, -1,		0.0, 1.0, 1.0,	 //7
};

// Define index data
// This is a list of indices referencing positions in the pointData
/*
std::vector<uint16_t> indexData = {
	0, 1, 2, // Triangle #0 connects points #0, #1 and #2
	0, 2, 3,  // Triangle #1 connects points #0, #2 and #3

	0, 1, 4,
	1, 2, 4,
	2, 3, 4,
	3, 0, 4
};
*/
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


float aa = 1;
int main(int _argc, char** _argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);

	wgfx::init(wgfx::getSurface(window), 1280, 720);

	wgfx::Program program = wgfx::loadProgram(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));
	wgfx::VertexBuffer vbo(pointData, 6);
	vbo.setAttribute(0, wgfx::vec3f, 0);
	vbo.setAttribute(1, wgfx::vec3f, 3); // take in a type
	wgfx::IndexBuffer ibo(indexData);

	float color[] = { 0.1, 0.2, 0.3, 0.4 };
	wgfx::Uniform uniform(0, sizeof(float), 1.0f);
	wgfx::Uniform uniform2(1, sizeof(color), color);

	glm::mat4 proj = glm::perspective(glm::radians(50.0f), float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(80.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

	wgfx::Uniform viewUniform(2, sizeof(glm::mat4), glm::value_ptr(view));
	wgfx::Uniform projUniform(3, sizeof(glm::mat4), glm::value_ptr(proj));

	wgfx::Uniform ratio(4, sizeof(float), 16.0 / 9.0);

	program.setUniform(uniform);				// how can we me make it more natural<< i mean, uniform object so that we can update it. but wgfx::setUniform <<  
	program.setUniform(uniform2);
	program.setUniform(viewUniform);
	program.setUniform(projUniform);
	program.setUniform(ratio);

	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	bool shouldClose = false;
	while (!shouldClose)
	{
		SDL_Event event;
		float t = SDL_GetTicks() / 1000.0f;
		float r = SDL_GetTicks() / 100.0f;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
			{
				int newWidth = event.window.data1;
				int newHeight = event.window.data2;
				float aspectRatio = (float)newWidth / (float)newHeight;

				program.updateUniform(ratio, aspectRatio);

				//wgfx::destroySurface();
				wgfx::initSurface();
				wgfx::initDepth(newWidth, newHeight);

				program.updateUniform(uniform, t);

				proj = glm::perspective(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);
				program.updateUniform(projUniform, glm::value_ptr(proj));
					//program.updateUniform(projUniform, glm::value_ptr(proj));

				// Update viewport and aspect ratio uniform
				//wgfx::setViewport(0, 0, newWidth, newHeight);
				//wgfx::init(wgfx::getSurface(window), newWidth, newHeight);
				//program.updateUniform(aspectRatioUniform, aspectRatio);
			}
			break;

			case SDL_EVENT_QUIT:
				shouldClose = true;
				break;

			case SDL_EVENT_WINDOW_EXPOSED:
				wgfx::initSurface();
				program.updateUniform(uniform, t);

			default:
				break;
			}
		}


		float d = 0.2;
		const Uint8* state = SDL_GetKeyboardState(NULL);

		if (state[SDL_SCANCODE_G]) {
			d = 0.4;
		}
		float cc[] = { 1.0, d, 1, 0.4 };
		view = glm::rotate(view, glm::radians(aa), glm::vec3(0.4f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

		program.updateUniform(uniform, t);
		program.updateUniform(uniform2, cc);

		//float v[16];
		//std::memcpy(v, glm::value_ptr(view), 16 * sizeof(float));

		program.updateUniform(viewUniform, glm::value_ptr(view));

		//program.updateUniform(viewUniform, v);
		//program.updateUniform(uniform2, r);

		wgfx::submit(program);

	}

}