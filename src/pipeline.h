#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include "uniform.h"
#include "buffer.h"

namespace wgfx
{
	struct Compute
	{
		ComputePipeline pipeline;
		ComputePipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;
		Uniforms uniforms;
		std::string entryPoint = "main";

		Compute()
		{
			std::cout << "Create compute pipeline\n";
			pipelineDesc = ComputePipelineDescriptor();
		}

		Uniform* addSampler(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createSampler(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setSampler(uniform);
			return uniform;
		}

		void updateUniform(int index, const float* data)
		{
			uniforms.updateUniform(uniforms.uniforms.at(index), data);
		}

		void updateUniform(Uniform* uniform, const float* data)
		{
			if (uniform == nullptr) return;
			if (uniforms.dynamicOffsets.size() <= static_cast<size_t>(uniform->binding)) {
				uniforms.dynamicOffsets.resize(static_cast<size_t>(uniform->binding) + 1, 0);
			}
			uniforms.dynamicOffsets.at(uniform->binding) = 0;
			uniform->quantity = 0;
			queue.writeBuffer(uniform->buffer, 0, data, uniform->minBindingSize);
		}

		Uniform* addUniform(int index)
		{
			wgfx::Uniform* uniform = wgfx::createUniform(index, sizeof(float) * 16, 1.0f);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setUniform(uniform);
			return uniform;
		}

		void setUniform(Uniform* uniform)
		{
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setUniform(uniform);
		}

		Uniform* addStorage(int index, size_t size, const void* data, bool readOnly = false)
		{
			wgfx::Uniform* uniform = wgfx::createStorage(index, size, data, readOnly);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setStorage(uniform);
			return uniform;
		}

		void setStorage(Uniform* storage)
		{
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setStorage(storage);
		}

		Uniform* addTexture(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createTexture(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setTexture(uniform);
			return uniform;
		}

		Uniform* addTexture3D_Uint(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createTexture3D_Uint(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setTexture(uniform);
			return uniform;
		}

		Uniform* addStorageTexture(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createTexture(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Compute;
			uniforms.setStorageTexture(uniform);
			return uniform;
		}

		void updateStorageBuffer(Uniform* storage, const void* data)
		{
			queue.writeBuffer(storage->buffer, 0, data, storage->minBindingSize);
		}

		void updateStorageBuffer(Uniform* storage, const void* data, size_t size, uint32_t offset)
		{
			queue.writeBuffer(storage->buffer, offset, data, size);
		}


		void init() // use me
		{
			// Create the bind group layout first
			uniforms.touch();
			
			// Create pipeline layout using the bind group layout
			PipelineLayoutDescriptor layoutDesc = {};
			layoutDesc.bindGroupLayoutCount = 1;
			layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&uniforms.bindGroupLayout;
			PipelineLayout pipelineLayout = device.createPipelineLayout(layoutDesc);
			
			pipelineDesc.layout = pipelineLayout;
			pipelineDesc.compute.module = shaderModule;
			pipelineDesc.compute.entryPoint = entryPoint.c_str();
			pipeline = device.createComputePipeline(pipelineDesc);
			std::cout << "Compute pipeline: " << pipeline << "\n";
			shaderModule.release();
		}


	};

	struct Pipeline
	{
		bool useDepth = true;
		int targets = 1;

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

		// Sub-objects referenced by pipelineDesc must outlive init() (no stack locals).
		VertexBufferLayout vertexBufferLayout{};
		FragmentState fragmentState{};
		ColorTargetState colorTargets[10]{};
		BlendState fragmentBlend{};
		DepthStencilState depthStencilState{};

		//std::vector<uint32_t> dynamicOffsets; // this also shouldn't be here.

		/*std::vector<Uniform*> getUniforms()
		{
			return uniforms.uniforms;
		}*/

		Uniform* getUniform(int index)
		{
			return uniforms.uniforms.at(index);
		}




		Pipeline();
								//BindGroup bindGroup;
		
					// delete me and abstract the other
					void setVertexBuffer(const std::vector<float>& value);
					void setIndexBuffer(const std::vector<uint16_t>& value);
					void setIndexBuffer(const std::vector<uint32_t>& value);

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
			
		// should put in a size of allocation
		// addUniform(int index, size_t size);

		/*
		ex:
		addUniform(5, sizeof(float) * 4)
		*/
		Uniform* addSampler(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createSampler(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Fragment;
			uniforms.setSampler(uniform);
			return uniform;
		}

		Uniform* addTexture(int index, wgfx::Texture texture)
		{
			wgfx::Uniform* uniform = wgfx::createTexture(index, texture);
			uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			uniforms.setTexture(uniform);
			return uniform;
		}

		Uniform* addUniform(int index)
		{
			wgfx::Uniform* uniform = wgfx::createUniform(index, sizeof(float) * 16, 1.0f);
			uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			uniforms.setUniform(uniform);
			return uniform;
		}

		void setUniform(Uniform* uniform)
		{
			uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			uniforms.setUniform(uniform);
		}
		
		Uniform* addStorage(int index, size_t size, const void* data)
		{
			wgfx::Uniform* uniform = wgfx::createStorage(index, size, data);
			uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			uniforms.setStorage(uniform);
			return uniform;
		}

		void setStorage(Uniform* storage)
		{
			uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			uniforms.setStorage(storage);
		}

		//void touch();


		// so here we update, but

		void updateUniform(int index, const float* data)
		{
			uniforms.updateUniform(uniforms.uniforms.at(index), data);
		}

		void updateUniform(Uniform* uniform, const float* data)
		{
			if (uniform == nullptr) return;
			if (uniforms.dynamicOffsets.size() <= static_cast<size_t>(uniform->binding)) {
				uniforms.dynamicOffsets.resize(static_cast<size_t>(uniform->binding) + 1, 0);
			}
			uniforms.dynamicOffsets.at(uniform->binding) = 0;
			uniform->quantity = 0;
			queue.writeBuffer(uniform->buffer, 0, data, uniform->minBindingSize);
		}


		void updateStorageBuffer(Uniform* storage, const void* data)
		{
			queue.writeBuffer(storage->buffer, 0, data, storage->minBindingSize);
		}

		void updateStorageBuffer(Uniform* storage, const void* data, size_t size, uint32_t offset)
		{
			queue.writeBuffer(storage->buffer, offset, data, size);
		}


	};

	inline std::vector<Pipeline*> pipelines;
	Pipeline* loadPipeline(std::string source);
	
	inline Compute* loadCompute(std::string source)
	{
		if (source.empty()) {
			std::cerr << "wgfx::loadCompute: empty shader source (WGSL file missing or unreadable)\n";
		}
		// Load the shader module
		ShaderModuleDescriptor shaderDesc;
#ifdef WEBGPU_BACKEND_WGPU
		shaderDesc.hintCount = 0;
		shaderDesc.hints = nullptr;
#endif

		// We use the extension mechanism to specify the WGSL part of the shader module descriptor
		ShaderModuleWGSLDescriptor shaderCodeDesc;
		// Set the chained struct's header
		shaderCodeDesc.chain.next = nullptr;
		shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
		// Connect the chain
		shaderDesc.nextInChain = &shaderCodeDesc.chain;
		shaderCodeDesc.code = source.c_str();
		ShaderModule shaderModule = device.createShaderModule(shaderDesc);

		Compute* pipeline = new Compute();
		pipeline->shaderModule = shaderModule;

		return pipeline;
	}

	static std::string loadFromFile(const std::filesystem::path& path) {
		std::error_code ec;
		std::filesystem::path absPath = std::filesystem::absolute(path, ec);
		if (ec) {
			absPath = path;
		}
		std::ifstream file(absPath, std::ios::binary);
		if (!file) {
			std::cerr << "wgfx::loadFromFile: cannot open \"" << absPath.string() << "\"\n";
			return {};
		}
		std::ostringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	
}