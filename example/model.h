#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <wgfx.h>

#include <array>

#include "maths.h"

struct Face
{
	std::array<float, 12> vertices;
	std::array<float, 12> normals;  // 4 normals, 3 coordinates each
	std::array<float, 8> uvs;       // 4 UVs, 2 coordinates each
};

const Face quads[Math::DIRECTION_COUNT] =
{
	{ {1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0}, { 1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0 } },
	{ {0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1}, {-1, 0, 0,   -1, 0, 0,   -1, 0, 0,   -1, 0, 0} },
	{ {1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1}, {0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0}, },
	{ {0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}, {0, -1, 0,   0, -1, 0,   0, -1, 0,   0, -1, 0}, },
	{ {1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1}, {0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1}, },
	{ {0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0}, {0, 0, -1,   0, 0, -1,   0, 0, -1,   0, 0, -1}, }
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
		// nothing seems to be happening here,
		/*
		but something ought to be happening here

		pipeline->setVertexBuffer(vbo_); << << << hmm it ought to update it. but at the same time...

		*/
		pipeline->setVertexBuffer(vbo_);
		pipeline->setIndexBuffer(ibo_);

		//pipeline->updateVertexBuffer(vertices_);
		//pipeline->updateIndexBuffer(indices_); //hmm hmm hm
	}

	void addData(std::vector<float> vertices, std::vector<uint16_t> indices)
	{
		// could it just be that we need to give vbo_ an index, of some kind. a unique identifier.
		// when create vbo give a hash,
		// then update it.


		//WE CREATE THE thing
		// then we udpate it, and 
		// trouble trouble double double


		vbo_ = wgfx::createVertexBuffer(vertices);
		vbo_->setAttribute(0, wgfx::vec3f, 0); // pos
		vbo_->setAttribute(1, wgfx::vec2f, 3); // norm
		vbo_->setAttribute(2, wgfx::vec3f, 6); // uv
		ibo_ = wgfx::createIndexBuffer(indices);
	}

	void clear()
	{
		vertices_.clear();
		indices_.clear();

		indexCount_ = 0;



		// destroy vbo and ibo! hmm..
	}

	int indexCount() const { return indexCount_; }

		std::array<float, 8> uv = { 1, 0, 0, 0, 0, 1, 1, 1 };
	void addQuad(
		const Face& face,
		glm::vec2 uvOffset,
		float uvScale,
		const glm::ivec3 pos,
		glm::ivec3 chunkpos
	)
	{
		int index = 0;
		int index2 = 0;
		int index3 = 0;
		for (int i = 0; i < 4; i++)
		{

			vertices_.push_back(face.vertices.at(index++) + pos.x + chunkpos.x);
			vertices_.push_back(face.vertices.at(index++) + pos.y + chunkpos.y);
			vertices_.push_back(face.vertices.at(index++) + pos.z + chunkpos.z);

			vertices_.push_back(face.normals.at(index2++));
			vertices_.push_back(face.normals.at(index2++));
			vertices_.push_back(face.normals.at(index2++));

			vertices_.push_back(uv.at(index3++) * uvScale + uvOffset.x);
			vertices_.push_back(uv.at(index3++) * uvScale + uvOffset.y);


		}

		/*int index2 = 0;
		for (int i = 0; i < 4; i++)
		{
			normals.push_back(face.normals.at(index2++));
			normals.push_back(face.normals.at(index2++));
			normals.push_back(face.normals.at(index2++));
		}*/
		indices_.push_back(indexCount_);
		indices_.push_back(indexCount_ + 1);
		indices_.push_back(indexCount_ + 2);
		indices_.push_back(indexCount_ + 2);
		indices_.push_back(indexCount_ + 3);
		indices_.push_back(indexCount_);
		indexCount_ += 4;
	}

	void buffer(Model* model)
	{
		model->addData(vertices_, indices_);
	}

	std::vector<float> vertices_;
	std::vector<uint16_t> indices_;
private:
	wgfx::VertexBuffer* vbo_;
	wgfx::IndexBuffer* ibo_;


	int indexCount_ = 0;

	void setupObjects(size_t reserve = 0)
	{
		vertices_.reserve(reserve);
		indices_.reserve(reserve * 3 / 2);
	}
};