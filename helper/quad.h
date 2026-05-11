#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "wgfx.h"
#include "constants.h"

class Quad
{
public:
	static Quad& Instance()
	{
		static Quad instance;
		return instance;
	}

	wgfx::Pipeline* pipeline;
	wgfx::Texture texture;
	std::vector<float> vertices;
	std::vector<uint16_t> indices;

	Quad()
	{
		pipeline = wgfx::loadPipeline(wgfx::loadFromFile((std::string(RESOURCE_DIR) + "/" + "quad.wgsl").c_str()));

		vertices = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f
		};

		indices = {0, 1, 2, 0, 2, 3};

		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
		vbo->setAttribute(0, wgfx::vec3f, 0);

		pipeline->setVertexBuffer(vertices);
		pipeline->setIndexBuffer(indices);

		texture = wgfx::loadTextureDst(raytraceWidth, raytraceHeight);
		pipeline->addTexture(0, texture);
		pipeline->addSampler(1, texture);

		pipeline->targets = 1;
		pipeline->useDepth = false;

		pipeline->init(vbo);
	}

	void render()
	{
		pipeline->uniforms.clear();
		pipeline->setVertexBuffer(vertices);
		pipeline->setIndexBuffer(indices);
	}

public:
	Quad(Quad const&) = delete;
	void operator=(Quad const&) = delete;
};
