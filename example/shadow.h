#pragma once

#include "shader.h"
#include "maths.h"

#include "cube.h"

#include "texture.h"

struct ShaderInfo
{
	glm::mat4 transform;
	wgfx::Texture* tex;
};

class Shadow
{
	Shader shader;
	wgfx::RenderPass pass; //

	glm::mat4 transform;
	wgfx::Texture tex;

public:
	Shadow()
	{
		shader = Shader("shadow.wgsl");

		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
			vbo->setAttribute(0, wgfx::vec3f, 0); // position
			vbo->setAttribute(1, wgfx::vec3f, 3); // color
			vbo->setAttribute(2, wgfx::vec2f, 6); // uv
		shader.setVertexBuffer({ 0.0 });
		shader.setIndexBuffer({ 0 }); // initial data point hmm 

		shader.setUniform(0); // view
		shader.setUniform(1); // model
		glm::mat4 proj = glm::perspective(glm::radians(90.0f)/*fov*/, float(1920) / 1080, 0.1f, 1000.0f); // reduce memory usage only send uniform for perspective once.
		shader.setUniform(2, proj); // proj
		shader.setTexture(3, texture); // tex wgfx::Uniform* sampler = wgfx::createTexture(3, texture);
		shader.setSampler(4, texture); //sampler  wgfx::Uniform* tex = wgfx::createSampler(4, texture);

		shader.pipeline->init(vbo);
		// hmm.

	}

	void render(const glm::mat4& cameraMatrix)
	{
		glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f));
		// compute light view matrix. (sun looking at origin)
		glm::mat4 lightView = glm::lookAt(-lightDir * 100.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// orthographic projection to cover scene in light
		float orthoSize = 100.0f;
		glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, -orthoSize, orthoSize);
		
		glm::mat4 lightSpaceMatrix = lightProj * lightView;

		// set
		transform = lightSpaceMatrix;


		// render depth into shadow map
		Cube::Instance().draw(transform, shader);

	}

	// recall, we need to have some
	// way to create a depth pipeline as well as a 
	// depth renderpass.

	// so it is sort of a depth shader.

	// and we need to create it before render.




	// getter for shaderInfo, just global for now.

	//ShaderInfo shaderInfo_;


};