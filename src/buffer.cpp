#include "buffer.h"

namespace wgfx
{
	VertexBuffer::VertexBuffer(std::vector<float> vertices)
	{
		initDepth();

		// Create vertex buffer
		bufferDesc.size = vertices.size() * sizeof(float);
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
		bufferDesc.mappedAtCreation = false;
		buffer = device.createBuffer(bufferDesc);

		// Upload geometry data to the buffer
		queue.writeBuffer(buffer, 0, vertices.data(), bufferDesc.size);

	}

	void VertexBuffer::setAttribute(int location, VertexFormat type, int offset)
	{
		int typeOffset = 0;
		if (type == wgfx::vec2f)
		{
			typeOffset = 2;
		}
		else if (type == wgfx::vec3f)
		{
			typeOffset = 3;
		}

		VertexAttribute attrib;
		attrib.shaderLocation = location;
		attrib.format = type;
		attrib.offset = offset * sizeof(float);

		vertexAttribs.emplace_back(attrib);
		this->fields += typeOffset;
	}

	IndexBuffer::IndexBuffer(std::vector<uint16_t> indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());

		bufferDesc.size = indices.size() * sizeof(uint16_t);
		bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
		buffer = device.createBuffer(bufferDesc);

		queue.writeBuffer(buffer, 0, indices.data(), bufferDesc.size);
	}
}