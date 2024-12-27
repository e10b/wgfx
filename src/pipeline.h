#pragma once

#include <filesystem>
#include <fstream>

#include "uniform.h"
#include "buffer.h"

namespace wgfx
{
	struct Pipeline
	{
		std::vector<Uniform*> uniforms;
		int dynamicUniformCount = 0;
		RenderPipeline pipeline;

		int index = 0;

		RenderPipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;

		//VertexBuffer* vertexBuffer;
		//IndexBuffer* indexBuffer;
		
		//std::vector<VertexBuffer*> vertexBuffers;
		//std::vector<IndexBuffer*> indexBuffers;
			// bleedin heart rightly mark thee.
		VertexBuffers vbos;
		IndexBuffers ibos;

		std::vector<uint32_t> dynamicOffsets;

		Pipeline();
			BindGroup bindGroup;
		
			void setVertexBuffer(const std::vector<float>& value);

			void setIndexBuffer(const std::vector<uint16_t>& value);

			void setVertexBuffer(VertexBuffer* vbo)
			{
				if (vbo->id == -1) {
					// If vbo->id is -1, assign it to the current size of the vector.
					vbo->id = vbos.vertexBuffers.size();
					vbos.vertexBuffers.push_back(vbo); // Add the new VBO to the vector.
				}
				else {
					// Ensure that vbo->id is within the bounds of the vector.
					if (vbo->id >= vbos.vertexBuffers.size()) {
						// If the id is out of bounds, resize the vector and fill with null pointers (or create default VBOs).
						vbos.vertexBuffers.resize(vbo->id + 1, nullptr);
					}
					vbos.vertexBuffers.at(vbo->id) = vbo;
				}
				//vbos.vertexBuffers.push_back(vbo);
				vbos.current = vbo;
			}

			void setIndexBuffer(IndexBuffer* ibo)
			{
				if (ibo->id == -1)
				{
					ibo->id = ibos.indexBuffers.size();
					ibos.indexBuffers.push_back(ibo);
				}
				else
				{
					if (ibo->id >= ibos.indexBuffers.size()) {
						ibos.indexBuffers.resize(ibo->id + 1, nullptr);
					}
					ibos.indexBuffers.at(ibo->id) = ibo;
				}
				ibos.current = ibo;
			}

		void init(VertexBuffer* vertexBuffer);
			BindGroupLayout bindGroupLayout;
			BindGroupLayoutDescriptor bindGroupLayoutDesc;
			std::vector<BindGroupLayoutEntry> entries;
			std::vector<BindGroupEntry> bindings;
			void setUniform(Uniform* uniform, bool dynamic);
		void setTexture(Uniform* uniform);
		void setSampler(Uniform* uniform);
		void touch();
		void updateUniform(Uniform* uniform, const float* array);


	};

	inline std::vector<Pipeline*> pipelines;
	Pipeline* loadPipeline(std::string source);
	
	static std::string loadFromFile(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) { return nullptr; }
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		std::string shaderSource(size, ' ');
		file.seekg(0);
		file.read(shaderSource.data(), size);
		return shaderSource;
	}

	
}