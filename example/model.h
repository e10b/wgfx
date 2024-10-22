#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <wgfx.h>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

class Model
{
public:
	Model(size_t reserve = 1000)
	{
		setupObjects(reserve);
	}

	Model(std::vector<float> vertices, std::vector<uint16_t> indices)
	{
		setupObjects();
		addData(vertices, indices);
	}

	void bind(wgfx::Pipeline* pipeline)
	{
		pipeline->setVertexBuffer(vbo_);
		pipeline->setIndexBuffer(ibo_); //hmm hmm hm
	}

	void addData(std::vector<float> vertices, std::vector<uint16_t> indices)
	{
		vbo_ = wgfx::createVertexBuffer(vertices);
		vbo_.setAttribute(0, wgfx::vec3f, 0); // pos
		vbo_.setAttribute(1, wgfx::vec3f, 3); // norm
		vbo_.setAttribute(2, wgfx::vec2f, 6); // uv
		ibo_ = wgfx::createIndexBuffer(indices);

	}

private:
	wgfx::VertexBuffer vbo_;
	wgfx::IndexBuffer ibo_;

	std::vector<Vertex> vertices_;
	std::vector<uint16_t> indices_;

	int indexCount_ = 0;

	void setupObjects(size_t reserve = 0)
	{
		vertices_.reserve(reserve);
		indices_.reserve(reserve * 3 / 2);
	}
};