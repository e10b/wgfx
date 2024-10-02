#define WGPU_IMPLEMENTATION
#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>


//The view in bgfx is analogous to the combination of a RenderPass(which controls the rendering context) and a TextureView(which defines the render target).


std::vector<float> pointData = {
	// x     y     z     r    g    b    u    v

	// Front face
	-1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
	 1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.9f,  1.0f, 0.0f, // Bottom-right
	-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
	 1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right
	 
	 // Back face
	 -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
	  1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.9f,  1.0f, 0.0f, // Bottom-right
	 -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
	  1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right

	  // Left face
	  -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
	  -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  1.0f, 0.0f, // Bottom-right
	  -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
	  -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-right

	  // Right face
	   1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.9f,  0.0f, 0.0f, // Bottom-left
	   1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.9f,  1.0f, 0.0f, // Bottom-right
	   1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, // Top-left
	   1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right

	   // Top face
	   -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
		1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Bottom-right
	   -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
		1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right

		// Bottom face
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.9f,  1.0f, 0.0f, // Bottom-right
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Top-left
		 1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 0.9f,  1.0f, 1.0f  // Top-right
};

std::vector<uint16_t> indexData = {
	// Front face
	0, 1, 2,
	1, 3, 2,

	// Back face
	4, 5, 6,
	5, 7, 6,

	// Left face
	8, 9, 10,
	9, 11, 10,

	// Right face
	12, 13, 14,
	13, 15, 14,

	// Top face
	16, 17, 18,
	17, 19, 18,

	// Bottom face
	20, 21, 22,
	21, 23, 22
};

int main(int _argc, char** _argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);
	wgfx::init(wgfx::getSurface(window));

	wgfx::Pipeline pipeline = wgfx::loadPipeline(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));

	wgfx::RenderPass renderPass;

	wgfx::VertexBuffer vbo(pointData);
		vbo.setAttribute(0, wgfx::vec3f, 0); // position
		vbo.setAttribute(1, wgfx::vec3f, 3); // color
		vbo.setAttribute(2, wgfx::vec2f, 6); // uv
	wgfx::IndexBuffer ibo(indexData);

	glm::mat4 proj = glm::perspective(glm::radians(60.0f)/*fov*/, float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	wgfx::Texture texture;

	wgfx::Uniform viewUniform = wgfx::loadUniform(0, sizeof(glm::mat4), 1.0f);					 pipeline.setUniform(viewUniform, true);
	wgfx::Uniform modelUniform = wgfx::loadUniform(1, sizeof(glm::mat4), 1.0f);				     pipeline.setUniform(modelUniform, true);
	wgfx::Uniform projUniform = wgfx::loadUniform(2, sizeof(glm::mat4), glm::value_ptr(proj));	 pipeline.setUniform(projUniform, false);

	wgfx::Uniform sampler = wgfx::loadTexture(3, texture);										 pipeline.setTexture(sampler, false);
	wgfx::Uniform actualsampler = wgfx::loadSampler(4, texture);									 pipeline.setSampler(actualsampler, false);

	
	pipeline.setVertexBuffer(vbo);
	pipeline.setIndexBuffer(ibo);

	bool close = false;// SDL_GL_SetSwapInterval(0); // Enable V-Sync
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
				wgfx::initSurface();
				wgfx::initDepth();

				int width, height;
				SDL_GetWindowSize(window, &width, &height);

				proj = glm::perspective(glm::radians(50.0f), float(width) / float(height), 0.1f, 100.0f);
				pipeline.updateUniform(projUniform, glm::value_ptr(proj));
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
		
		renderPass.touch();

		pipeline.updateUniform(viewUniform, glm::value_ptr(view));

		// draw cubes
		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				float* mtx = glm::value_ptr(rotationMatrix);
				pipeline.updateUniform(modelUniform, mtx);
				
				renderPass.draw(pipeline);
			}
		}

		renderPass.end();
		wgfx::frame();
	}

}