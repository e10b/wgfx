#pragma once

#include "texture.h"

namespace wgfx
{
	struct Uniform
	{
		bool isDepth = false;

		Buffer buffer;
		//BindGroupEntry binding;
		BindGroupEntry entry;
		//int index;
		int binding;
			//size_t scale;
		int minBindingSize;

		int offset;

		int stride;
		int quantity = 0; // uncrease when hmm, 

		Uniform() = default;

	};

	Uniform* createUniform(int i, size_t size, float data);
	Uniform* createUniform(int i, size_t size, const float* array);
	Uniform* createTexture(int i, Texture texture);
	Uniform* createSampler(int i, Texture texture);


	inline Uniform* createTexture2(int i, wgpu::TextureView t)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.textureView = t;

		return uniform;
	}

	inline Uniform* createSampler2(int i, wgpu::Sampler s)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.sampler = s;

		return uniform;
	}

	// right, so we have a << hmm, a container.
	
	class Uniforms
	{
	public:
		// this is an object which should allow for containment.
		// so what is in now,
		std::vector<Uniform*> uniforms;
		int dynamicUniformCount = 0;

		std::vector<uint32_t> dynamicOffsets;

		//std::vector<BindGroupLayoutEntry> entries;
		//std::vector<BindGroupEntry> bindings;

		std::vector<BindGroupLayoutEntry> layouts; // frmrly entries
		std::vector<BindGroupEntry> entries; // frmrly bindings

		BindGroup bindGroup;
		BindGroupLayout bindGroupLayout;
		BindGroupLayoutDescriptor bindGroupLayoutDesc;

		void clear()
		{
			//if (reset) {
				for (auto uniform : uniforms)
				{
					uniform->quantity = 0;
				}
			//}
			//reset = false;
		}

		void touch()
		{
			bindGroupLayoutDesc.entryCount = layouts.size();
			bindGroupLayoutDesc.entries = layouts.data();
			bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

			BindGroupDescriptor bindGroupDesc;
			bindGroupDesc.layout = bindGroupLayout;
			bindGroupDesc.entryCount = entries.size();
			bindGroupDesc.entries = entries.data();
			bindGroup = device.createBindGroup(bindGroupDesc);
		}

		void updateUniform(Uniform* uniform, const float* array)
		{
			auto& current = *uniforms.at(uniform->binding);

			const uint32_t offset = current.quantity * current.stride;

			if (dynamicOffsets.size() <= uniform->binding) {
				dynamicOffsets.resize(uniform->binding + 1, 0);
			}
			dynamicOffsets[uniform->binding] = offset;

			queue.writeBuffer(current.buffer, offset, array, current.minBindingSize);
			current.quantity++;
		}

		//hmm
			void setUniform(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = {};
					layout.binding = uniform->binding;
					layout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
					layout.buffer.type = BufferBindingType::Uniform;
					layout.buffer.minBindingSize = uniform->minBindingSize;
					layout.buffer.hasDynamicOffset = true; // dynamic
					dynamicUniformCount++;
					dynamicOffsets.push_back(0); // default value
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void setTexture(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = {};							/// layout needs to be created in joint with the actual entry
					layout.binding = uniform->binding;
					layout.visibility = ShaderStage::Fragment;
					//layout.texture.sampleType = TextureSampleType::Float;

					if (uniform->isDepth) // Add a boolean flag in Uniform for this
					{
						layout.texture.sampleType = TextureSampleType::Depth;
					}
					else
					{
						layout.texture.sampleType = TextureSampleType::Float;
					}


					layout.texture.viewDimension = TextureViewDimension::_2D;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void setSampler(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = {};
					layout.binding = uniform->binding;
					layout.visibility = ShaderStage::Fragment;
					layout.sampler.type = SamplerBindingType::Filtering;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

		
		//set in the pipe should be quite simple really just a call to the container.
		// 
		// Pipeline::setUniform(int uniform)
		// {
		//		uniforms.setUniform(uniform);	
		//		^^ frankely we do not need abstraction for the operation, instead we can 
		// }
		// 
		// else
		// Pipeline::setUniform(int uniform)
		// {
		// 
		// }
		// 
		// 
		//massive sod. right, 
	};

	// right, so I think that dynamics should be automatic.
	
}