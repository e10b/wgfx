#pragma once

#include <glm/glm.hpp>

#include <vector>

#include <wgfx.h>

#include "shader.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model
{
public:
	Model(size_t reserve = 100);

	Model(std::vector<float> vertices, std::vector<uint16_t> indices);

	void addData(std::vector<float> vertices, std::vector<uint16_t> indices);

	void bind(Shader* shader);

private:
	wgfx::VertexBuffer vbo_;
	wgfx::IndexBuffer ibo_;

	std::vector<float> vertices_;
	std::vector<uint16_t> indices_;

	int indexCount_ = 0;

	void setupObjects(size_t reserve = 0);
};