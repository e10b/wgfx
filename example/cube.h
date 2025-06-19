#pragma once

#include "shader.h"
#include "model.h"

std::vector<float> pointData =
{
	// x    y    z    nx   ny   nz   u    v
	// Back face
	1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
	0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
	1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,

	// Right face
	1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

	// Front face
	0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
	1.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
	1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

	// Left face
	0.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

	// Top face
	0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
	1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

	// Bottom face
	0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f
};
std::vector<uint16_t> indexData =
{
		0, 1, 2,
		2, 3, 0,

		// Right face
		4, 5, 6,
		6, 7, 4,

		// Front face
		8, 9, 10,
		10, 11, 8,

		// Left face
		12, 13, 14,
		14, 15, 12,

		// Top face
		16, 17, 18,
		18, 19, 16,

		// Bottom face
		20, 21, 22,
		22, 23, 20
};

#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

class Cube
{
	Shader shader;
	Model mesh;
	wgfx::Texture texture;

public:

	static Cube& Instance()
	{
		static Cube instance;
		return instance;
	}

	Cube(Cube const&) = delete;
	void operator=(Cube const&) = delete;

	Cube()
	{

		shader = Shader("cube.wgsl");
		mesh = Model(pointData, indexData);
		texture = wgfx::loadTexture(RESOURCE_DIR "/crate.png");

		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
			vbo->setAttribute(0, wgfx::vec3f, 0); // position
			vbo->setAttribute(1, wgfx::vec3f, 3); // color
			vbo->setAttribute(2, wgfx::vec2f, 6); // uv
		shader.setVertexBuffer({ 0.0 });
		shader.setIndexBuffer({ 0 }); // initial data point hmm 

		//shader.pipeline.setVertexBuffer(vbo);

		//wgfx::IndexBuffer* ibo = wgfx::createIndexBuffer(indexData);
		//shader.pipeline.setIndexBuffer(ibo);

			shader.setUniform(0); // view
			shader.setUniform(1); // model
			glm::mat4 proj = glm::perspective(glm::radians(90.0f)/*fov*/, float(1920) / 1080, 0.1f, 1000.0f); // reduce memory usage only send uniform for perspective once.
			shader.setUniform(2, proj); // proj
			shader.setTexture(3, texture); // tex wgfx::Uniform* sampler = wgfx::createTexture(3, texture);
			shader.setSampler(4, texture); //sampler  wgfx::Uniform* tex = wgfx::createSampler(4, texture);

			shader.pipeline->init(vbo);
		//shader.pipeline.setTexture(sampler);
		//shader.pipeline.setSampler(tex);


		//shader.pipeline.init(); // auto init?? well, 
	}

	void drawLit(const Camera& camera, wgfx::RenderPass& pass)
	{
		shader.touch();

		glm::mat4 cameraMatrix = camera.getViewMatrix();

		draw(cameraMatrix, shader, pass);
	}

	//void draw(const Camera& camera, wgfx::RenderPass& pass)
	void draw(const glm::mat4& cameraMatrix, Shader& shader, wgfx::RenderPass& pass)
	{
		float time = SDL_GetTicks() / 1000.0f; // ought not

		//shader.renderPass.touch();
		shader.updateUniform(0, cameraMatrix);
		//shader.updateUniform(2, camera.getProjectionMatrix());

		//for (uint32_t zz = 0; zz < 11; ++zz) {
		//	for (uint32_t yy = 0; yy < 21; ++yy) {
		//		for (uint32_t xx = 0; xx < 11; ++xx) {
		//			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
		//			rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
		//			rotationMatrix = glm::rotate(rotationMatrix, time + zz * 0.45f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis (new rotation for zz)

		//			rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, -15.0f + float(zz) * 3.0f, 1.0f); // Adjust position for zz

		//			shader.updateUniform(1, rotationMatrix);
		//			mesh.bind(shader.pipeline);
		//			pass.draw(shader.pipeline);
		//		}
		//	}
		//}
		
		//floor
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				glm::mat4 identity = glm::mat4(1.0f);
				identity = glm::translate(identity, glm::vec3(i, 0, j));
				shader.updateUniform(1, identity);
				mesh.bind(shader.pipeline); 
				pass.draw(shader.pipeline);
				//shader.renderPass.draw(shader.pipeline);

			}
		}

		//tower
		for (int i = 0; i < 5; i++) 
		{
				glm::mat4 identity = glm::mat4(1.0f);
				identity = glm::translate(identity, glm::vec3(5, 1, 5));
				identity = glm::translate(identity, glm::vec3(0, i, 0));
				shader.updateUniform(1, identity);
				mesh.bind(shader.pipeline);
				pass.draw(shader.pipeline);
				//shader.use();
				//shader.renderPass.draw(shader.pipeline);
		}

		shader.end();
	}
};





