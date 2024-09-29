#define WGPU_IMPLEMENTATION
#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>


//The view in bgfx is analogous to the combination of a RenderPass(which controls the rendering context) and a TextureView(which defines the render target).


std::vector<float> pointData = {
	// x     y     z      r    g    b     u    v
	-1.0, -1.0,  1.0,	0.0, 0.0, 0.0,	0.0, 0.0, // Bottom-left front
	 1.0, -1.0,  1.0,	0.0, 0.0, 0.9,	0.9, 0.0, // Bottom-right front
	-1.0,  1.0,  1.0,	0.0, 1.0, 0.0,	0.0, 1.0, // Top-left front
	 1.0,  1.0,  1.0,	0.0, 1.0, 1.0,	0.9, 1.0, // Top-right front
	-1.0, -1.0, -1.0,   1.0, 0.0, 0.0,  0.0, 0.0, // Bottom-left back
	 1.0, -1.0, -1.0,   1.0, 0.0, 0.9,  0.9, 0.0, // Bottom-right back
	-1.0,  1.0, -1.0,   1.0, 1.0, 0.0,  0.0, 1.0, // Top-left back
	 1.0,  1.0, -1.0,   1.0, 1.0, 1.0,  1.0, 1.0  // Top-right back
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
	wgfx::init(wgfx::getSurface(window));

	wgfx::Pipeline pipeline = wgfx::loadPipeline(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));

	wgfx::RenderPass renderPass;

	wgfx::VertexBuffer vbo(pointData);
		vbo.setAttribute(0, wgfx::vec3f, 0); // position
		vbo.setAttribute(1, wgfx::vec3f, 3); // color
		vbo.setAttribute(2, wgfx::vec2f, 6); // uv
	wgfx::IndexBuffer ibo(indexData);

	glm::mat4 proj = glm::perspective(glm::radians(60.0f)/*fov*/, float(1920) / 1080, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -35.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	wgfx::Texture tex;

	wgfx::DynamicUniform viewUniform(0, sizeof(glm::mat4), 1.0f);				  pipeline.setUniform(viewUniform, false);
	wgfx::DynamicUniform projUniform(1, sizeof(glm::mat4), glm::value_ptr(proj)); pipeline.setUniform(projUniform, false);
	wgfx::DynamicUniform modelUniform(2, sizeof(glm::mat4), 1.0f);				  pipeline.setUniform(modelUniform, true);

	wgfx::DynamicUniform sampler(3, tex); pipeline.setTexture(sampler);

	pipeline.setVertexBuffer(vbo);
	pipeline.setIndexBuffer(ibo);

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


		// I think we will throw the program to the fbo(renderPass)...
		// 
		// one way to do it is to make
		// 
		// RenderPass renderPass
		// 
		// renderPass.setProgram(program);
		// renderPass.setVertexBuffer(vbo);
		// renderPass.draw();
		// 
		// renderPass.end();
		// 
		// 
		// 
		// /// further i think i want to rename program to pipeline just to stick with the wgpu naming convention..
		// 
		// wgfx::frame();
		// 
							//SOMETHING LIKE THIS//fbo.setProgram(program); 

		//wgfx::touch(&fbo); //wgfx::touch(0); << the view
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
		
		wgfx::frame(); // good
	}

}