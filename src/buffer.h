#pragma once

#include "surface.h"

namespace wgfx
{
	struct VertexBuffer
	{
		std::vector<VertexAttribute> vertexAttribs;
		Buffer buffer;
		int fields = 0;

		std::vector<float> data;

		VertexBuffer() {};

		//VertexBuffer(std::vector<float> vertices);
		void setAttribute(int location, VertexFormat type, int offset);
		
		
	};

	VertexBuffer* createVertexBuffer(std::vector<float> vertices = { 0.0 });

	struct IndexBuffer
	{
		Buffer buffer;
		uint32_t indexCount;

		std::vector<uint16_t> data;

		IndexBuffer() : buffer(nullptr), indexCount(0) {} // allow for no index buff

		
		//IndexBuffer(std::vector<uint16_t> indices);
	};

	IndexBuffer* createIndexBuffer(std::vector<uint16_t> indices = {0});

	
}