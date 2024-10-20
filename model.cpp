#include "model.h"

Model::Model(size_t reserve)
{
	setupObjects(reserve);
}

Model::Model(std::vector<float> vertices, std::vector<uint16_t> indices)
{
	setupObjects();
	addData(vertices, indices);
}

void Model::addData(std::vector<float> vertices, std::vector<uint16_t> indices)
{
	vbo_ = wgfx::createVertexBuffer(vertices);
		vbo_.setAttribute(0, wgfx::vec3f, 0); // position
		vbo_.setAttribute(1, wgfx::vec3f, 3); // color
		vbo_.setAttribute(2, wgfx::vec2f, 6); // uv
	ibo_ = wgfx::createIndexBuffer(indices);

}

void Model::bind(Shader* shader)
{
	shader->pipeline.setVertexBuffer(vertices_);
	shader->pipeline.setIndexBuffer(indices_);
}

void Model::setupObjects(size_t reserve)
{
	vertices_.reserve(reserve);
	indices_.reserve(reserve);
}