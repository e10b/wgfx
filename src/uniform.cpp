#include "uniform.h"

namespace wgfx
{
	static uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
		uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
		return step * divide_and_ceil;
	}

Uniform* createStorage(int i, size_t size, const void* data, bool readOnly)
{
    std::cout << "Creating storage buffer with binding " << i << " and size " << size << " bytes..." << std::endl;
    
    Uniform* storage = new Uniform();
    storage->binding = i;
    storage->minBindingSize = size;
    storage->isReadOnly = readOnly;

    // For storage buffers, stride and offset are unused
    storage->stride = 0;
    storage->offset = 0;

    BufferDescriptor bufferDesc;
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage | BufferUsage::CopySrc;  // Use CopySrc for copying to read buffer
    bufferDesc.mappedAtCreation = false;

    storage->buffer = device.createBuffer(bufferDesc);
    std::cout << "Storage buffer created: " << storage->buffer << std::endl;

    if (data != nullptr) {
        queue.writeBuffer(storage->buffer, 0, data, size);
        std::cout << "Data written to storage buffer" << std::endl;
    }

    storage->entry.binding = i;
    storage->entry.buffer = storage->buffer;
    storage->entry.offset = 0;
    storage->entry.size = size;

    return storage;
}


	Uniform* createUniform(int i, size_t size, float data)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;
		uniform->minBindingSize = size;
		// all uniforms are currently large enough for dynamics but shouldn't be if not dynamic
		uint32_t uniformStride = ceilToNextMultiple(
			(uint32_t)size,
			(uint32_t)deviceLimits.minUniformBufferOffsetAlignment
		);// intervalic sizing - scaled to the offset alignment val
		uniform->stride = uniformStride;
		bufferDesc.size = 2560 * uniformStride + size; // max size
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
		bufferDesc.mappedAtCreation = false;
		uniform->buffer = device.createBuffer(bufferDesc);

		queue.writeBuffer(uniform->buffer, 0, &data, size);

		uniform->entry.binding = i;
		uniform->entry.buffer = uniform->buffer;
		uniform->entry.offset = 0;
		uniform->entry.size = size;

		return uniform;
	}
	Uniform* createUniform(int i, size_t size, const float* array)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;
		uniform->minBindingSize = size;

		uint32_t uniformStride = ceilToNextMultiple(
			(uint32_t)size,
			(uint32_t)deviceLimits.minUniformBufferOffsetAlignment
		);
		uniform->stride = uniformStride;
		bufferDesc.size = 2560 * uniformStride + size;
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
		bufferDesc.mappedAtCreation = false;
		uniform->buffer = device.createBuffer(bufferDesc);

		// Use writeBuffer with the pointer to float data
		queue.writeBuffer(uniform->buffer, 0, array, size);

		uniform->entry.binding = i;
		uniform->entry.buffer = uniform->buffer;
		uniform->entry.offset = 0;
		uniform->entry.size = size;
		return uniform;
	}
	Uniform* createTexture(int i, const Texture& texture)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.textureView = texture.textureView;

		return uniform;
	}

	Uniform* createDepthTexture(int i, TextureView textureView)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;
		uniform->isDepth = true;

		uniform->entry.binding = i;
		uniform->entry.textureView = textureView;

		return uniform;
	}

	Uniform* createTexture3D_Uint(int i, const Texture& texture)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;
		uniform->isUint = true;
		uniform->is3D = true;

		uniform->entry.binding = i;
		uniform->entry.textureView = texture.textureView;

		return uniform;
	}

	Uniform* createSampler(int i, const Texture& texture)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.sampler = texture.sampler;

		return uniform;
	}

	Uniform* createComparisonSampler(int i)
	{
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::ClampToEdge;
		samplerDesc.addressModeV = AddressMode::ClampToEdge;
		samplerDesc.addressModeW = AddressMode::ClampToEdge;
		samplerDesc.magFilter = FilterMode::Linear;
		samplerDesc.minFilter = FilterMode::Linear;
		samplerDesc.mipmapFilter = MipmapFilterMode::Nearest;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 1.0f;
		samplerDesc.compare = CompareFunction::Less;
		samplerDesc.maxAnisotropy = 1;

		Uniform* uniform = new Uniform();
		uniform->binding = i;
		uniform->isComparisonSampler = true;
		uniform->entry.binding = i;
		uniform->entry.sampler = device.createSampler(samplerDesc);
		return uniform;
	}

}
