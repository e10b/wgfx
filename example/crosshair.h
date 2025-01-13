#pragma once

#include <glm/glm.hpp>

#include "shader.h"
#include "model.h"

class Crosshair
{
public:
	Crosshair()
		:shader_("crosshair.wgsl")
	{
		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
		vbo->setAttribute(0, wgfx::vec3f, 0); // pos
		shader_.setVertexBuffer({ 0.0 });
		shader_.setIndexBuffer({ 0 });

		shader_.setUniform(0); // size

		shader_.pipeline->init(vbo);
	}

	void render(glm::vec2 size)
	{
		shader_.touch();
		//shader_.updateUniform(0, size); // size

		shader_.updateUniform(0, size);

		cross_.addData({ -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f }
		, { 0,1,2,3,2,1 });

		cross_.bind(shader_.pipeline);
		shader_.use();
		shader_.end();
	}

	Shader shader_;
private:
	Model cross_;
};