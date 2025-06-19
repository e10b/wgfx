#pragma once

#include <filesystem>
#include <fstream>

#include "uniform.h"
#include "buffer.h"

namespace wgfx
{
	
	//todo
	// need a pipeline config of some kind so that i can have useDepth etc, in the configuration for now see useDepth
	// e.g. we need a multisample value in the config
	// then get rid of the auto msample in the renderpass for post proces.



	struct Pipeline
	{
		bool useDepth = true;

		bool multiTarget = true;

		//std::vector<Uniform*> uniforms; // this is the issue
		//int dynamicUniformCount = 0; // likewise
		RenderPipeline pipeline; // should be fine

		int index = 0; // bad notation

		RenderPipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;

							//VertexBuffer* vertexBuffer;
							//IndexBuffer* indexBuffer;
							//std::vector<VertexBuffer*> vertexBuffers;
							//std::vector<IndexBuffer*> indexBuffers;
			// bleedin heart rightly mark thee.
		Uniforms uniforms;
		VertexBuffers vbos;
		IndexBuffers ibos;

		//std::vector<uint32_t> dynamicOffsets; // this also shouldn't be here.

		Pipeline();
								//BindGroup bindGroup;
		
					// delete me and abstract the other
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
			//BindGroupLayout bindGroupLayout;
			//BindGroupLayoutDescriptor bindGroupLayoutDesc;
			//std::vector<BindGroupLayoutEntry> entries;
			//std::vector<BindGroupEntry> bindings;
			
			
			//void setUniform(Uniform* uniform, bool dynamic);
			void setUniform(Uniform* uniform)
			{
				uniforms.setUniform(uniform);
			}


			void setTexture(Uniform* uniform)
			{
				uniforms.setTexture(uniform);
			}
			void setSampler(Uniform* uniform)
			{
				uniforms.setSampler(uniform);
			}
		void touch();
		//void updateUniform(Uniform* uniform, const float* array);
		void updateUniform(Uniform* uniform, const float* array)
		{
			uniforms.updateUniform(uniform, array);
		}

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