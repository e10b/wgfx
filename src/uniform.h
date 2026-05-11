#pragma once

#include "texture.h"
#include <algorithm>

namespace wgfx
{
	struct Uniform
	{
		bool isDepth = false;
		bool isUint = false;
		bool is3D = false;
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
		int dynamicOffsetIndex = -1;

		Uniform() = default;

	};

	Uniform* createStorage(int i, size_t size, const void* data, bool readOnly = false);
	Uniform* createUniform(int i, size_t size, float data);
	Uniform* createUniform(int i, size_t size, const float* array);
	Uniform* createTexture(int i, Texture texture);
	Uniform* createTexture3D_Uint(int i, Texture texture);
	Uniform* createSampler(int i, Texture texture);

	inline void copyUniformToUniform(Uniform* first, Uniform* second)
	{
		encoder.copyBufferToBuffer(first->buffer, 0, second->buffer, 0, sizeof(float) * 4);
		//hardcoded quantity for now
	}

	inline void copyTextureToTexture(Texture* first, Texture* second)
	{
		wgpu::ImageCopyTexture src = {};
		src.texture = first->texture;
		src.mipLevel = 0;
		src.origin = { 0, 0, 0 };
		src.aspect = wgpu::TextureAspect::All;

		wgpu::ImageCopyTexture dst = {};
		dst.texture = second->texture;
		dst.mipLevel = 0;
		dst.origin = { 0, 0, 0 };
		dst.aspect = wgpu::TextureAspect::All;


		wgpu::Extent3D extent = {};
		extent.width = first->width;
		extent.height = first->height;
		extent.depthOrArrayLayers = 1;
		encoder.copyTextureToTexture(src, dst, extent);
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

		ShaderStageFlags visibility;
		uint64_t lastUpdateFrame = 0;

		void clear()
		{
			for (auto uniform : uniforms)
			{
				uniform->quantity = 0;
			}
			std::fill(dynamicOffsets.begin(), dynamicOffsets.end(), 0);
		}

		void clearForNewFrame()
		{
			if (lastUpdateFrame != frameIndex) {
				clear();
				lastUpdateFrame = frameIndex;
			}
		}

		void touch()
		{
			std::cout << "Creating bind group layout with " << layouts.size() << " entries..." << std::endl;
			bindGroupLayoutDesc.entryCount = layouts.size();
			bindGroupLayoutDesc.entries = layouts.data();
			bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
			std::cout << "Bind group layout created: " << bindGroupLayout << std::endl;

			std::cout << "Creating bind group with " << entries.size() << " entries..." << std::endl;
			BindGroupDescriptor bindGroupDesc;
			bindGroupDesc.layout = bindGroupLayout;
			bindGroupDesc.entryCount = entries.size();
			bindGroupDesc.entries = entries.data();
			bindGroup = device.createBindGroup(bindGroupDesc);
			std::cout << "Bind group created: " << bindGroup << std::endl;
		}

		void updateUniform(Uniform* uniform, const float* array)
		{
			clearForNewFrame();

			Uniform* current = uniforms.at(uniform->binding);

			int dynamicOffset = current->quantity * current->stride;
			
			if (uniform->dynamicOffsetIndex < 0) return;
			if (dynamicOffsets.size() <= static_cast<size_t>(uniform->dynamicOffsetIndex)) { dynamicOffsets.resize(uniform->dynamicOffsetIndex + 1, 0); }
			dynamicOffsets.at(uniform->dynamicOffsetIndex) = dynamicOffset;

			queue.writeBuffer(current->buffer, dynamicOffset, array, uniform->minBindingSize);

			current->quantity++;
		}

		void updateStorageBuffer(Uniform* storage, const void* data, size_t size, size_t offset = 0)
		{
			if (storage == nullptr) return;
			if (size + offset > storage->minBindingSize) {
				throw std::runtime_error("updateStorage: Write exceeds storage buffer capacity.");
			}

			queue.writeBuffer(storage->buffer, offset, data, size);
		}

		//hmm
			void setUniform(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = Default;
					layout.binding = uniform->binding;
					layout.visibility = visibility;
					layout.buffer.type = BufferBindingType::Uniform;
					layout.buffer.minBindingSize = uniform->minBindingSize;
					layout.buffer.hasDynamicOffset = true; // dynamic
					uniform->dynamicOffsetIndex = dynamicUniformCount;
					dynamicUniformCount++;
					dynamicOffsets.push_back(0); // default value
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}


			void setStorageTexture(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = Default;							/// layout needs to be created in joint with the actual entry
				layout.binding = uniform->binding;
				layout.visibility = visibility;

				layout.storageTexture.access = StorageTextureAccess::WriteOnly;
				layout.storageTexture.format = TextureFormat::RGBA8Unorm;
				layout.storageTexture.viewDimension = TextureViewDimension::_2D;

				//layout.texture.viewDimension = TextureViewDimension::_2D;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void setTexture(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = Default;							/// layout needs to be created in joint with the actual entry
					layout.binding = uniform->binding;
					layout.visibility = visibility;
					
					// Set sample type based on texture format
					if (uniform->isDepth) {
						layout.texture.sampleType = TextureSampleType::Depth;
					}
					else if (uniform->isUint) {
						layout.texture.sampleType = TextureSampleType::Uint;
					}
					else {
						layout.texture.sampleType = TextureSampleType::Float;
					}
					
					// Set view dimension based on texture type
					if (uniform->is3D) {
						layout.texture.viewDimension = TextureViewDimension::_3D;
					}
					else {
						layout.texture.viewDimension = TextureViewDimension::_2D;
					}
					
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}



			void setTextureArray(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = Default;
					layout.binding = uniform->binding;
					layout.visibility = ShaderStage::Fragment;
					layout.texture.sampleType = TextureSampleType::Float;
					layout.texture.viewDimension = TextureViewDimension::_2DArray;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}

			void setSampler(Uniform* uniform)
			{
				BindGroupLayoutEntry layout = Default;
					layout.binding = uniform->binding;
					layout.visibility = visibility;
					layout.sampler.type = SamplerBindingType::Filtering;
				uniforms.push_back(uniform);
				layouts.push_back(layout);
				entries.push_back(uniform->entry);
			}


			void setStorage(Uniform* storage)
			{
				BindGroupLayoutEntry layout = Default;
				layout.binding = storage->binding;
				layout.visibility = visibility; // Compute shader visibility
				if (storage->isReadOnly) {
					layout.buffer.type = BufferBindingType::ReadOnlyStorage;
				} else {
					layout.buffer.type = BufferBindingType::Storage;
				}
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