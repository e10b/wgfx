#pragma once

#include "shader.h"
#include "model.h"

std::vector<float> pointData =
{
	// x    y      z		nx    ny    nz     u     v
	-1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,  // Front face
	 1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,  // Back face
	 1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,  // Left face
	-1.0f, -1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,

	 1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,  // Right face
	 1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f,

	-1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,  // Top face
	 1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,  // Bottom face
	 1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f
};
std::vector<uint16_t> indexData =
{
	0, 1, 2,
	1, 3, 2,
	4, 5, 6,
	5, 7, 6,
	8, 9, 10,
	9, 11, 10,
	12, 13, 14,
	13, 15, 14,
	16, 17, 18,
	17, 19, 18,
	20, 21, 22,
	21, 23, 22
};

#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

class Cube
{
	Shader shader;
	Model mesh;
	wgfx::Texture texture;

public:

	Cube()
	{

		shader = Shader("ballsagain");
		mesh = Model(pointData, indexData);
		texture = wgfx::loadTexture(RESOURCE_DIR "/crate2.jpg");

		wgfx::VertexBuffer vbo = wgfx::createVertexBuffer(pointData);
		vbo.setAttribute(0, wgfx::vec3f, 0); // position
		vbo.setAttribute(1, wgfx::vec3f, 3); // color
		vbo.setAttribute(2, wgfx::vec2f, 6); // uv
		shader.pipeline.setVertexBuffer(vbo);

		wgfx::IndexBuffer ibo = wgfx::createIndexBuffer(indexData);
		shader.pipeline.setIndexBuffer(ibo);

		shader.setUniform(0); // view
		shader.setUniform(1); // model
		shader.setUniform(2); // proj
		shader.setTexture(3, texture); // tex wgfx::Uniform* sampler = wgfx::createTexture(3, texture);
		shader.setSampler(4, texture); //sampler  wgfx::Uniform* tex = wgfx::createSampler(4, texture);

		//shader.pipeline.setTexture(sampler);
		//shader.pipeline.setSampler(tex);


		shader.pipeline.init(); // auto init?? well, 
	}

	void draw(const Camera& camera)
	{
		float time = SDL_GetTicks() / 1000.0f; // ought not

		shader.renderPass.touch();
		shader.updateUniform(0, camera.getViewMatrix());
		shader.updateUniform(2, camera.getProjectionMatrix());
		
		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				shader.updateUniform(1, rotationMatrix);

				shader.use();
			}
		}
		shader.renderPass.end();
	}
};