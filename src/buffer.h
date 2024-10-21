#pragma once

#include "surface.h"

namespace wgfx
{
	struct VertexBuffer
	{
		std::vector<VertexAttribute> vertexAttribs;
		Buffer buffer;
		int fields = 0;

		VertexBuffer() {};

		//VertexBuffer(std::vector<float> vertices);
		void setAttribute(int location, VertexFormat type, int offset);
		
		
	};

	VertexBuffer createVertexBuffer(std::vector<float> vertices);

	struct IndexBuffer
	{
		Buffer buffer;
		uint32_t indexCount;

		IndexBuffer() : buffer(nullptr), indexCount(0) {} // allow for no index buff

		
		//IndexBuffer(std::vector<uint16_t> indices);
	};

	IndexBuffer createIndexBuffer(std::vector<uint16_t> indices);

}