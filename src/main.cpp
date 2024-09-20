#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<float> pointData = {
		-1, -1,  1,		0.0, 0.0, 0.0,
		 1, -1,  1,		0.0, 0.0, 1.0,
		-1,  1,  1,		0.0, 1.0, 0.0,
		 1,  1,  1,		0.0, 1.0, 1.0,
		-1, -1, -1,		1.0, 0.0, 0.0,
		 1, -1, -1,		1.0, 0.0, 1.0,
		-1,  1, -1,		1.0, 1.0, 0.0,
		 1,  1, -1,		1.0, 1.0, 1.0,
};

std::vector<uint16_t> indexData = {
		2, 6, 7,
		2, 3, 7,
		0, 4, 5,
		0, 1, 5,
		0, 2, 6,
		0, 4, 6,
		1, 3, 7,
		1, 5, 7,
		0, 2, 3,
		0, 1, 3,
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
	wgfx::VertexBuffer vbo(pointData, 6);
	vbo.setAttribute(0, wgfx::vec3f, 0); // position
	vbo.setAttribute(1, wgfx::vec3f, 3); // color

	glm::mat4 proj = glm::perspective(glm::radians(60.0f)/*fov*/, float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -35.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	wgfx::DynamicUniform viewUniform(0, sizeof(glm::mat4), 1.0f);
	wgfx::DynamicUniform projUniform(1, sizeof(glm::mat4), glm::value_ptr(proj));
	wgfx::DynamicUniform modelUniform(2, sizeof(glm::mat4), 1.0f);

	program.setUniform(viewUniform, false);
	program.setUniform(projUniform, false);
	program.setUniform(modelUniform, true);

	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	bool shouldClose = false;
	while (!shouldClose)
	{
		SDL_Event event;
		float time = SDL_GetTicks() / 1000.0f;
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
				//program.updateUniform(projUniform, glm::value_ptr(proj));
			}
			break;

			case SDL_EVENT_QUIT:
				shouldClose = true;
				break;

			case SDL_EVENT_KEY_DOWN:
				if (event.key.key == SDLK_ESCAPE) {
					shouldClose = true; // Close the application if Escape is pressed
				}
				break;

			case SDL_EVENT_WINDOW_EXPOSED:
				wgfx::initSurface();

			default:
				break;
			}
		}


		wgfx::touch(program);

		program.updateUniform(viewUniform, glm::value_ptr(view));

		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				float* mtx = glm::value_ptr(rotationMatrix);
				program.updateUniform(modelUniform, (mtx));
				wgfx::draw(program);
			}
		}

		for (auto uniform : program.dynamicUniforms)
		{
			uniform->quantity = 0;
		}

		wgfx::frame();
	}

}