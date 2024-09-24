#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>


//The view in bgfx is analogous to the combination of a RenderPass(which controls the rendering context) and a TextureView(which defines the render target).


std::vector<float> pointData = {
	//	 x	 y	 z		r	 g	  b
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

	wgfx::VertexBuffer vbo(pointData, 6);
	vbo.setAttribute(0, wgfx::vec3f, 0); // position
	vbo.setAttribute(1, wgfx::vec3f, 3); // color
	wgfx::IndexBuffer ibo(indexData);

	glm::mat4 proj = glm::perspective(glm::radians(60.0f)/*fov*/, float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -35.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	wgfx::wTexture tex;
	tex.dothing();
	wgfx::DynamicUniform viewUniform(0, sizeof(glm::mat4), 1.0f);				  program.setUniform(viewUniform, false, false);
	wgfx::DynamicUniform projUniform(1, sizeof(glm::mat4), glm::value_ptr(proj)); program.setUniform(projUniform, false, false);
	wgfx::DynamicUniform modelUniform(2, sizeof(glm::mat4), 1.0f);				  program.setUniform(modelUniform, true, false);

	wgfx::DynamicUniform sampler(tex, 3); program.setUniform(sampler, false, true);

	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	bool close = false;
	while (!close)
	{
		SDL_Event event;
		float time = SDL_GetTicks() / 1000.0f;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
			{
				int w = event.window.data1;
				int h = event.window.data2;
				float aspectRatio = (float)w / (float)h;

				wgfx::initSurface();
				wgfx::initDepth(w, h);

				proj = glm::perspective(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);
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

		wgfx::View mainView;
		// Set clear color to blue, depth to 1.0, and stencil to 0
		mainView.setViewClear(glm::vec4(0.188f, 0.188f, 0.188f, 1.0f));

		// Set the view matrix (camera) and projection matrix
			//glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			//glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
			//mainView.setViewTransform(viewMatrix, projMatrix);

		wgfx::touch(mainView); //wgfx::touch(0); << the view

		program.updateUniform(viewUniform, glm::value_ptr(view));

		// draw cubes
		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				float* mtx = glm::value_ptr(rotationMatrix);
				program.updateUniform(modelUniform, (mtx));
				wgfx::draw(program); //wgfx::submit(0, m_program); << the view and the program..
			}
		}

		wgfx::frame(mainView); // good
	}

}