#include "buffer.h"

namespace wgfx
{
	/*VertexBuffer::VertexBuffer(std::vector<float> vertices)
	{
		createVertexBuffer(vertices);
	}*/

	VertexBuffer* createVertexBuffer(std::vector<float> vertices)
	{
		VertexBuffer* buffer = new VertexBuffer(); // heap really shouldn't matter..
		buffer->data = vertices;
		//initDepth();

		// Create vertex buffer
		BufferDescriptor bufferDesc;
			bufferDesc.size = vertices.size() * sizeof(float);
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
			bufferDesc.mappedAtCreation = true;
		buffer->buffer = device.createBuffer(bufferDesc);

		void* map = buffer->buffer.getMappedRange(0, vertices.size() * sizeof(float));
		memcpy(map, vertices.data(), vertices.size() * sizeof(float));
		buffer->buffer.unmap();

		// Upload geometry data to the buffer
		//queue.writeBuffer(buffer->buffer, 0, vertices.data(), vertices.size() * sizeof(float));
		// occurs at write
		//std::cout << "balls\n";

		return buffer;
	}

	void VertexBuffer::setAttribute(int location, VertexFormat type, int offset)
	{
		int typeOffset = 0;
		if (type == wgfx::vec1f)
		{
			typeOffset = 1;
		}
		else if (type == wgfx::vec2f)
		{
			typeOffset = 2;
		}
		else if (type == wgfx::vec3f)
		{
			typeOffset = 3;
		}
		else if (type == wgfx::vec4f)
		{
			typeOffset = 4;
		}

		VertexAttribute attrib;
		attrib.shaderLocation = location;
		attrib.format = type;
		attrib.offset = offset * sizeof(float);

		vertexAttribs.emplace_back(attrib);
		this->fields += typeOffset;
	}

	/*IndexBuffer::IndexBuffer(std::vector<uint16_t> indices)
	{
		createIndexBuffer(indices);
	}*/

	IndexBuffer* createIndexBuffer(std::vector<uint16_t> indices)
	{
		IndexBuffer* buffer = new IndexBuffer();
		buffer->data = indices;
		buffer->is32Bit = false;

		buffer->indexCount = static_cast<uint32_t>(indices.size());

		bufferDesc.size = indices.size() * sizeof(uint16_t);
		bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
		buffer->buffer = device.createBuffer(bufferDesc);

		queue.writeBuffer(buffer->buffer, 0, indices.data(), bufferDesc.size);

		return buffer;
	}

	IndexBuffer* createIndexBufferU32(std::vector<uint32_t> indices)
	{
		IndexBuffer* buffer = new IndexBuffer();
		buffer->data32 = indices;
		buffer->is32Bit = true;
		buffer->indexCount = static_cast<uint32_t>(indices.size());

		bufferDesc.size = indices.size() * sizeof(uint32_t);
		bufferDesc.size = (bufferDesc.size + 3) & ~3u;
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
		buffer->buffer = device.createBuffer(bufferDesc);

		queue.writeBuffer(buffer->buffer, 0, indices.data(), bufferDesc.size);

		return buffer;
	}

	IndexBuffer* createIndexBuffer(std::vector<uint32_t> indices)
	{
		return createIndexBufferU32(std::move(indices));
	}
}