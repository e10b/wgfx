#pragma once

#include "model.h"
#include "shader.h"

#include "camera.h"
#include "texture.h"

#include "chunk.h"
#include "terrain.h"



class Test
{
public:
	Test()
		: shader_("shader.wgsl"),
		chunk_({0,0}),
		chunk2_({1, 0})
	{
		texture = wgfx::loadTexture(RESOURCE_DIR "/stone.png");
		chunk_.generate(terrain_);
		chunk2_.generate(terrain_);
		
		chunk_.buildMesh();
		chunk2_.buildMesh();
	
		wgfx::VertexBuffer vbo = wgfx::createVertexBuffer();
		vbo.setAttribute(0, wgfx::vec3f, 0); // position
		vbo.setAttribute(1, wgfx::vec3f, 3); // color
		vbo.setAttribute(2, wgfx::vec2f, 6); // uv
		shader_.pipeline.setVertexBuffer(vbo);

		wgfx::IndexBuffer ibo = wgfx::createIndexBuffer(indexData);
		shader_.pipeline.setIndexBuffer(ibo);

		shader_.setUniform(0); // camera
		shader_.setUniform(1); // camera
		shader_.setTexture(2, texture); // tex
		shader_.setSampler(3, texture); // sampler
		shader_.setUniform(4); // camera

		shader_.pipeline.init(); // auto init?? well, 

	}

	void render(const Camera& cam)
	{
		shader_.renderPass.touch();

		shader_.updateUniform(0, cam.getMatrix());

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.f));
		shader_.updateUniform(1, model);
		
		shader_.pipeline.updateVertexBuffer(chunk_.model_.vertices_);
		shader_.pipeline.updateIndexBuffer(chunk_.model_.indices_);
		//chunk_.draw(&shader_.pipeline);
		shader_.use();



		glm::mat4 model2 = glm::translate(glm::mat4(1.0f), chunk2_.getWorldPos() + glm::vec3(0.0f, 0.f, 0.f));
		shader_.updateUniform(1, model2);
		shader_.pipeline.updateVertexBuffer(chunk2_.model_.vertices_);
		shader_.pipeline.updateIndexBuffer(chunk2_.model_.indices_);
		shader_.use();
		

		shader_.renderPass.end();
		
	}

private:
	Chunk chunk_;
	Chunk chunk2_;
	Terrain terrain_;

	Shader shader_;
	wgfx::Texture texture;
};
