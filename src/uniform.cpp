#include "uniform.h"

namespace wgfx
{
	static uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
		uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
		return step * divide_and_ceil;
	}
	Uniform* createStorage(int i, size_t size, const void* data, bool readOnly)
	{
		Uniform* storage = new Uniform();
		storage->binding = i;
		storage->minBindingSize = static_cast<int>(size);
		storage->isReadOnly = readOnly;
		storage->stride = 0;
		storage->offset = 0;

		BufferDescriptor localBufferDesc{};
		localBufferDesc.size = size;
		localBufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage | BufferUsage::CopySrc;
		localBufferDesc.mappedAtCreation = false;
		storage->buffer = device.createBuffer(localBufferDesc);

		if (data != nullptr && size > 0) {
			queue.writeBuffer(storage->buffer, 0, data, size);
		}

		storage->entry.binding = i;
		storage->entry.buffer = storage->buffer;
		storage->entry.offset = 0;
		storage->entry.size = size;
		return storage;
	}
	Uniform* createUniform(int i, size_t size, float data)
	{
		// Debug: Log uniform buffer creation
		std::cout << "Creating uniform buffer " << i << " with size " << size << std::endl;
		
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

		std::cout << "Created uniform buffer with total size: " << bufferDesc.size << " bytes" << std::endl;

		queue.writeBuffer(uniform->buffer, 0, &data, size);

		uniform->entry.binding = i;
		uniform->entry.buffer = uniform->buffer;
		uniform->entry.offset = 0;
		uniform->entry.size = size;

		return uniform;
	}	Uniform* createUniform(int i, size_t size, const float* array)
	{
		// Debug: Log uniform buffer creation
		std::cout << "Creating uniform buffer " << i << " with size " << size << " (array)" << std::endl;
		
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

		std::cout << "Created uniform buffer with total size: " << bufferDesc.size << " bytes" << std::endl;

		// Use writeBuffer with the pointer to float data
		queue.writeBuffer(uniform->buffer, 0, array, size);

		uniform->entry.binding = i;
		uniform->entry.buffer = uniform->buffer;
		uniform->entry.offset = 0;
		uniform->entry.size = size;
		return uniform;
	}
	Uniform* createTexture(int i, Texture texture)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.textureView = texture.textureView;

		return uniform;
	}
	Uniform* createSampler(int i, Texture texture)
	{
		Uniform* uniform = new Uniform();
		uniform->binding = i;

		uniform->entry.binding = i;
		uniform->entry.sampler = texture.sampler;

		return uniform;
	}

}