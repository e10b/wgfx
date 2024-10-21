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

		RenderPipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		std::vector<uint32_t> dynamicOffsets;

		Pipeline();
			BindGroup bindGroup;
		void setIndexBuffer(IndexBuffer buffer);
		void updateIndexBuffer(std::vector<uint16_t> indices)
		{
			IndexBuffer buffer = wgfx::createIndexBuffer(indices);
			indexBuffer = buffer;
		}
		void updateVertexBuffer(std::vector<float> vertices)
		{
			VertexBuffer buffer = wgfx::createVertexBuffer(vertices);
			vertexBuffer = buffer;
		}
		void init();
			BindGroupLayout bindGroupLayout;
			BindGroupLayoutDescriptor bindGroupLayoutDesc;
			std::vector<BindGroupLayoutEntry> entries;
			std::vector<BindGroupEntry> bindings;
			void setVertexBuffer(VertexBuffer);
		void setUniform(Uniform uniform, bool dynamic);
		void setTexture(Uniform uniform);
		void setSampler(Uniform uniform);
		void touch();
		void updateUniform(Uniform uniform, const float* array);

	};

	inline std::vector<Pipeline*> pipelines;
	Pipeline loadPipeline(std::string source);
	
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