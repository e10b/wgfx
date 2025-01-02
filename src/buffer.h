#pragma once

#include "surface.h"
#include <unordered_map>

namespace wgfx
{


	class VertexBuffer
	{
	public:
		std::vector<VertexAttribute> vertexAttribs;
		Buffer buffer;
		int fields = 0;
		int id = -1;
		std::vector<float> data;

		VertexBuffer() {};

		~VertexBuffer()
		{
			//if (!buffer) return;
			if (buffer != nullptr)
			{
				buffer.destroy();
				buffer.release();
			}
		}

		//VertexBuffer(std::vector<float> vertices);
		void setAttribute(int location, VertexFormat type, int offset);

		void write(std::vector<float> vertices)
		{
			/*size_t dataSize = vertices.size() * sizeof(float);
			if (dataSize > buffer.getSize())
			{
				std::cout << "too small!\n";
				return;
			}*/

			queue.writeBuffer(buffer, 0, vertices.data(), vertices.size() * sizeof(float));
		}
	};

	class VertexBuffers
	{
	public:
		std::vector<VertexBuffer*> vertexBuffers;

		VertexBuffer* current;

		void add(VertexBuffer* buffer)
		{
			vertexBuffers.push_back(buffer);
		}
		
	};

	VertexBuffer* createVertexBuffer(std::vector<float> vertices = { 0.0 });

	class IndexBuffer
	{
	public:
		Buffer buffer;
		uint32_t indexCount;
		int id = -1;

		std::vector<uint16_t> data;

		IndexBuffer() : buffer(nullptr), indexCount(0) {} // allow for no index buff
	};

	class IndexBuffers
	{
	public:
		std::vector<IndexBuffer*> indexBuffers;

		IndexBuffer* current;
		void add(IndexBuffer* buffer)
		{
			indexBuffers.push_back(buffer);
		}
	};

	IndexBuffer* createIndexBuffer(std::vector<uint16_t> indices = {0});

	
}