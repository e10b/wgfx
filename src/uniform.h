#pragma once

#include "texture.h"
#include <stdexcept>

namespace wgfx
{	struct Uniform
	{
		bool isDepth = false;
		bool isReadOnly = false;
		bool ownsBuffer = true;

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

		~Uniform()
		{
			if (ownsBuffer && buffer) {
				buffer.destroy();
				buffer.release();
			}
		}

	};

	Uniform* createUniform(int i, size_t size, float data);
	Uniform* createUniform(int i, size_t size, const float* array);
	Uniform* createStorage(int i, size_t size, const void* data = nullptr, bool readOnly = false);
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
		WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;

		BindGroup bindGroup;
		BindGroupLayout bindGroupLayout;
		BindGroupLayoutDescriptor bindGroupLayoutDesc;

		// Destructor to clean up WebGPU resources
		~Uniforms()
		{
			if (bindGroup) {
				bindGroup.release();
				bindGroup = nullptr;
			}
			if (bindGroupLayout) {
				bindGroupLayout.release();
				bindGroupLayout = nullptr;
			}
		}
		void clear()
		{
			// Always reset uniform quantities to prevent buffer overflow
			for (auto uniform : uniforms)
			{
				uniform->quantity = 0;
			}
		}
		void touch()
		{
			// Release previous bind group and layout if they exist
			if (bindGroup) {
				bindGroup.release();
				bindGroup = nullptr;
			}
			if (bindGroupLayout) {
				bindGroupLayout.release();
				bindGroupLayout = nullptr;
			}

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

		void updateStorageBuffer(Uniform* storage, const void* data, size_t size, size_t offset = 0)
		{
			if (storage == nullptr) return;
			if (size + offset > static_cast<size_t>(storage->minBindingSize)) {
				throw std::runtime_error("updateStorageBuffer: write exceeds storage buffer capacity");
			}
			queue.writeBuffer(storage->buffer, offset, data, size);
		}

		//hmm
			void setUniform(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = {};
					layout.binding = uniform->binding;
					layout.visibility = visibility;
					layout.buffer.type = BufferBindingType::Uniform;
					layout.buffer.minBindingSize = uniform->minBindingSize;
					layout.buffer.hasDynamicOffset = true; // dynamic
					dynamicUniformCount++;
					dynamicOffsets.push_back(0); // default value
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void updateTexture(Uniform* uniform, wgpu::TextureView newView)
			{
				uniform->entry.textureView = newView;

				// Also update the entry in the master entry list
				for (auto& entry : entries)
				{
					if (entry.binding == uniform->binding)
					{
						entry.textureView = newView;
						break;
					}
				}

				// Recreate the bind group with the updated texture view
				touch();
			}



			void setTexture(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = {};							/// layout needs to be created in joint with the actual entry
					layout.binding = uniform->binding;
					layout.visibility = visibility;
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
					layout.visibility = visibility;
					layout.sampler.type = SamplerBindingType::Filtering;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void setStorage(Uniform* storage)
			{
				BindGroupLayoutEntry layout = {};
				layout.binding = storage->binding;
				layout.visibility = visibility;
				layout.buffer.type = storage->isReadOnly ? BufferBindingType::ReadOnlyStorage : BufferBindingType::Storage;
				layout.buffer.minBindingSize = storage->minBindingSize;
				layout.buffer.hasDynamicOffset = false;
				uniforms.push_back(storage);
				layouts.push_back(layout);
				entries.push_back(storage->entry);
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