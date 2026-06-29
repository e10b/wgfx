/*
 * Copyright (c) 2026 Ethan Herman (e10b). All rights reserved.
 *
 * This source code is part of the wgfx graphics library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * For commercial licensing inquiries, proprietary integrations, or alternate
 * use-cases that fall outside the scope of the AGPLv3, please contact:
 * ethan34787@gmail.com
 */

#include "texture.h"

#include <algorithm>
#include <stb_image.h>

namespace wgfx
{
	// Auxiliary function for loadTexture
	static void writeMipMaps(Device device, wgpu::Texture texture, Extent3D textureSize, uint32_t mipLevelCount, const unsigned char* pixelData)
	{
		Queue queue = device.getQueue();

		// Arguments telling which part of the texture we upload to
		ImageCopyTexture destination;
		destination.texture = texture;
		destination.origin = { 0, 0, 0 };
		destination.aspect = TextureAspect::All;

		// Arguments telling how the C++ side pixel memory is laid out
		TextureDataLayout source;
		source.offset = 0;

		// Create image data
		Extent3D mipLevelSize = textureSize;
		std::vector<unsigned char> previousLevelPixels;
		Extent3D previousMipLevelSize;
		for (uint32_t level = 0; level < mipLevelCount; ++level) {
			// Pixel data for the current level
			std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
			if (level == 0) {
				// We cannot really avoid this copy since we need this
				// in previousLevelPixels at the next iteration
				memcpy(pixels.data(), pixelData, pixels.size());
			}
			else {
				// Create mip level data
				for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
					for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
						unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
						// Get the corresponding 4 pixels from the previous level
						unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
						unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
						unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
						unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
						// Average
						p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
						p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
						p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
						p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
					}
				}
			}

			// Upload data to the GPU texture
			destination.mipLevel = level;
			source.bytesPerRow = 4 * mipLevelSize.width;
			source.rowsPerImage = mipLevelSize.height;
			queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

			previousLevelPixels = std::move(pixels);
			previousMipLevelSize = mipLevelSize;
			mipLevelSize.width /= 2;
			mipLevelSize.height /= 2;
		}

		queue.release();
	}
	// Equivalent of std::bit_width that is available from C++20 onward
	static uint32_t bit_width(uint32_t m) {
		if (m == 0) return 0;
		else { uint32_t w = 0; while (m >>= 1) ++w; return w; }
	}

	Texture loadTexture(const std::filesystem::path& path)
	{
		Texture texture;

		// Create a sampler
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::Repeat;
		samplerDesc.addressModeV = AddressMode::Repeat;
		samplerDesc.addressModeW = AddressMode::Repeat;
		samplerDesc.magFilter = FilterMode::Linear;
		samplerDesc.minFilter = FilterMode::Linear;
		samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 8.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		// create a texture
		texture.textureView = nullptr;
		wgpu::Texture tex = makeTexture(path, device, &texture.textureView);
		if (!texture.textureView)
		{
			std::cerr << "Could not laod texture!\n";
		}
		std::cout << "Texture: " << tex << std::endl;
		std::cout << "Texture view: " << texture.textureView << std::endl;
		return texture;
	}

	Texture loadTextureSrc(int width, int height)
	{
		Texture texture;

		wgpu::TextureDescriptor texA = {};
		texA.size.width = width; texture.width = width;
		texA.size.height = height; texture.height = height;
		texA.size.depthOrArrayLayers = 1;
		texA.format = wgpu::TextureFormat::RGBA8Unorm;
		texA.mipLevelCount = 1;
		texA.sampleCount = 1;
		texA.dimension = wgpu::TextureDimension::_2D;
		texA.usage = wgpu::TextureUsage::TextureBinding | TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc;
		// this is specifically for copysrc
		wgpu::Texture computeTarget = device.createTexture(texA);
		wgpu::TextureView computeTargetView = computeTarget.createView();

		texture.texture = computeTarget;
		texture.textureView = computeTargetView;

		return texture;
	}

	Texture loadTextureDst(int width, int height)
	{
		Texture texture;

		// Create a sampler
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::Repeat;
		samplerDesc.addressModeV = AddressMode::Repeat;
		samplerDesc.addressModeW = AddressMode::Repeat;
		samplerDesc.magFilter = FilterMode::Nearest;//FilterMode::Linear;
		samplerDesc.minFilter = FilterMode::Nearest;//FilterMode::Linear;
		samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 8.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		wgpu::TextureDescriptor texA = {};
		texA.size.width = width; texture.width = width;
		texA.size.height = height; texture.height = height;
		texA.size.depthOrArrayLayers = 1;
		texA.format = wgpu::TextureFormat::RGBA8Unorm;
		texA.mipLevelCount = 1;
		texA.sampleCount = 1;
		texA.dimension = wgpu::TextureDimension::_2D;
		texA.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
		// this is specifically for copysrc
		wgpu::Texture computeTarget = device.createTexture(texA);
		wgpu::TextureView computeTargetView = computeTarget.createView();

		texture.texture = computeTarget;
		texture.textureView = computeTargetView;

		return texture;
	}

	Texture loadTexture3D(int width, int height, int depth, wgpu::TextureFormat format)
	{
		Texture texture;

		// Create a sampler for 3D texture
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::ClampToEdge;
		samplerDesc.addressModeV = AddressMode::ClampToEdge;
		samplerDesc.addressModeW = AddressMode::ClampToEdge;
		samplerDesc.magFilter = FilterMode::Nearest; // Important: use Nearest for exact voxel lookup
		samplerDesc.minFilter = FilterMode::Nearest;
		samplerDesc.mipmapFilter = MipmapFilterMode::Nearest;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 1.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		wgpu::TextureDescriptor texDesc = {};
		texDesc.size.width = width; texture.width = width;
		texDesc.size.height = height; texture.height = height;
		texDesc.size.depthOrArrayLayers = depth;
		texDesc.format = format;
		texDesc.mipLevelCount = 1;
		texDesc.sampleCount = 1;
		texDesc.dimension = wgpu::TextureDimension::_3D;
		texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
		
		wgpu::Texture tex3D = device.createTexture(texDesc);
		
		// Create 3D texture view
		wgpu::TextureViewDescriptor viewDesc = {};
		viewDesc.format = format;
		viewDesc.dimension = wgpu::TextureViewDimension::_3D;
		viewDesc.baseMipLevel = 0;
		viewDesc.mipLevelCount = 1;
		viewDesc.baseArrayLayer = 0;
		viewDesc.arrayLayerCount = 1;
		wgpu::TextureView tex3DView = tex3D.createView(viewDesc);

		texture.texture = tex3D;
		texture.textureView = tex3DView;

		return texture;
	}

	Texture loadTextureArray(const std::vector<std::filesystem::path>& paths)
	{
		Texture texture;

		if (paths.empty()) {
			std::cerr << "No texture paths provided for texture array!" << std::endl;
			return texture;
		}

		// Load the first texture to get dimensions
		int width, height, channels;
		unsigned char* firstPixelData = stbi_load(paths[0].string().c_str(), &width, &height, &channels, 4);
		if (!firstPixelData) {
			std::cerr << "Failed to load first texture: " << paths[0] << std::endl;
			return texture;
		}

		// Create sampler
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::Repeat;
		samplerDesc.addressModeV = AddressMode::Repeat;
		samplerDesc.addressModeW = AddressMode::Repeat;
		samplerDesc.magFilter = FilterMode::Nearest;
		samplerDesc.minFilter = FilterMode::Nearest;
		samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 8.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		// Create texture array descriptor
		TextureDescriptor textureDesc;
		textureDesc.dimension = TextureDimension::_2D;
		textureDesc.format = TextureFormat::RGBA8Unorm;
		textureDesc.size = { (unsigned int)width, (unsigned int)height, (unsigned int)paths.size() };
		textureDesc.mipLevelCount = bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
		textureDesc.sampleCount = 1;
		textureDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
		textureDesc.viewFormatCount = 0;
		textureDesc.viewFormats = nullptr;
		wgpu::Texture wgpuTexture = device.createTexture(textureDesc);

		Queue queue = device.getQueue();

		// Load and upload each texture layer
		for (size_t i = 0; i < paths.size(); ++i) {
			unsigned char* pixelData;
			if (i == 0) {
				pixelData = firstPixelData; // Use already loaded first texture
			} else {
				int w, h, c;
				pixelData = stbi_load(paths[i].string().c_str(), &w, &h, &c, 4);
				if (!pixelData) {
					std::cerr << "Failed to load texture: " << paths[i] << std::endl;
					continue;
				}
				if (w != width || h != height) {
					std::cerr << "Texture " << paths[i] << " has different dimensions than first texture!" << std::endl;
					stbi_image_free(pixelData);
					continue;
				}
			}

			// Upload texture data for this layer with mipmaps
			ImageCopyTexture destination;
			destination.texture = wgpuTexture;
			destination.origin = { 0, 0, (uint32_t)i };
			destination.aspect = TextureAspect::All;

			TextureDataLayout source;
			source.offset = 0;

			// Generate mipmaps for this layer
			Extent3D mipLevelSize = { (uint32_t)width, (uint32_t)height, 1 };
			std::vector<unsigned char> previousLevelPixels;
			Extent3D previousMipLevelSize;

			for (uint32_t level = 0; level < textureDesc.mipLevelCount; ++level) {
				std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
				if (level == 0) {
					memcpy(pixels.data(), pixelData, pixels.size());
				} else {
					// Generate mip level data
					for (uint32_t x = 0; x < mipLevelSize.width; ++x) {
						for (uint32_t y = 0; y < mipLevelSize.height; ++y) {
							unsigned char* p = &pixels[4 * (y * mipLevelSize.width + x)];
							unsigned char* p00 = &previousLevelPixels[4 * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 0))];
							unsigned char* p01 = &previousLevelPixels[4 * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 1))];
							unsigned char* p10 = &previousLevelPixels[4 * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 0))];
							unsigned char* p11 = &previousLevelPixels[4 * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 1))];
							p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
							p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
							p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
							p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
						}
					}
				}

				destination.mipLevel = level;
				source.bytesPerRow = 4 * mipLevelSize.width;
				source.rowsPerImage = mipLevelSize.height;
				queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

				previousLevelPixels = std::move(pixels);
				previousMipLevelSize = mipLevelSize;
				mipLevelSize.width /= 2;
				mipLevelSize.height /= 2;
			}

			if (i > 0) {
				stbi_image_free(pixelData);
			}
		}

		stbi_image_free(firstPixelData);
		queue.release();

		// Create texture view for the array
		TextureViewDescriptor textureViewDesc;
		textureViewDesc.aspect = TextureAspect::All;
		textureViewDesc.baseArrayLayer = 0;
		textureViewDesc.arrayLayerCount = paths.size();
		textureViewDesc.baseMipLevel = 0;
		textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
		textureViewDesc.dimension = TextureViewDimension::_2DArray;
		textureViewDesc.format = textureDesc.format;
		texture.textureView = wgpuTexture.createView(textureViewDesc);

		std::cout << "Texture array created with " << paths.size() << " layers" << std::endl;
		return texture;
	}

	Texture loadTexture2DArrayFromRgba8(int width, int height, const std::vector<std::vector<unsigned char>>& layersRgba,
	    bool srgbEncoded)
	{
		Texture texture;
		if (width < 1 || height < 1) {
			std::cerr << "loadTexture2DArrayFromRgba8: invalid dimensions\n";
			return texture;
		}

		const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
		const uint32_t layerCountU = static_cast<uint32_t>(std::max<size_t>(1, layersRgba.size()));

		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::Repeat;
		samplerDesc.addressModeV = AddressMode::Repeat;
		samplerDesc.addressModeW = AddressMode::Repeat;
		samplerDesc.magFilter = FilterMode::Linear;
		samplerDesc.minFilter = FilterMode::Linear;
		samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 12.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		TextureDescriptor textureDesc;
		textureDesc.dimension = TextureDimension::_2D;
		textureDesc.format = srgbEncoded ? TextureFormat::RGBA8UnormSrgb : TextureFormat::RGBA8Unorm;
		textureDesc.size = { (unsigned int)width, (unsigned int)height, layerCountU };
		textureDesc.mipLevelCount = bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
		textureDesc.sampleCount = 1;
		textureDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
		textureDesc.viewFormatCount = 0;
		textureDesc.viewFormats = nullptr;
		wgpu::Texture wgpuTexture = device.createTexture(textureDesc);
		texture.texture = wgpuTexture;
		texture.width = width;
		texture.height = height;

		Queue queue = device.getQueue();

		for (uint32_t i = 0; i < layerCountU; ++i) {
			std::vector<unsigned char> whiteFallback(expected, 255u);
			const unsigned char* pixelData = whiteFallback.data();
			if (!layersRgba.empty() && i < layersRgba.size() && layersRgba[i].size() >= expected) {
				pixelData = layersRgba[i].data();
			}

			ImageCopyTexture destination;
			destination.texture = wgpuTexture;
			destination.origin = { 0, 0, i };
			destination.aspect = TextureAspect::All;

			TextureDataLayout source;
			source.offset = 0;

			Extent3D mipLevelSize = { (uint32_t)width, (uint32_t)height, 1 };
			std::vector<unsigned char> previousLevelPixels;
			Extent3D previousMipLevelSize;

			for (uint32_t level = 0; level < textureDesc.mipLevelCount; ++level) {
				std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
				if (level == 0) {
					memcpy(pixels.data(), pixelData, pixels.size());
				} else {
					for (uint32_t x = 0; x < mipLevelSize.width; ++x) {
						for (uint32_t y = 0; y < mipLevelSize.height; ++y) {
							unsigned char* p = &pixels[4 * (y * mipLevelSize.width + x)];
							unsigned char* p00 = &previousLevelPixels[4 * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 0))];
							unsigned char* p01 = &previousLevelPixels[4 * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 1))];
							unsigned char* p10 = &previousLevelPixels[4 * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 0))];
							unsigned char* p11 = &previousLevelPixels[4 * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 1))];
							p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
							p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
							p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
							p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
						}
					}
				}

				destination.mipLevel = level;
				source.bytesPerRow = 4 * mipLevelSize.width;
				source.rowsPerImage = mipLevelSize.height;
				queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

				previousLevelPixels = std::move(pixels);
				previousMipLevelSize = mipLevelSize;
				mipLevelSize.width /= 2;
				mipLevelSize.height /= 2;
			}
		}

		queue.release();

		TextureViewDescriptor textureViewDesc;
		textureViewDesc.aspect = TextureAspect::All;
		textureViewDesc.baseArrayLayer = 0;
		textureViewDesc.arrayLayerCount = layerCountU;
		textureViewDesc.baseMipLevel = 0;
		textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
		textureViewDesc.dimension = TextureViewDimension::_2DArray;
		textureViewDesc.format = textureDesc.format;
		texture.textureView = wgpuTexture.createView(textureViewDesc);

		std::cout << "Texture 2D array (" << (srgbEncoded ? "RGBA8 sRGB" : "RGBA8 unorm") << ") created: " << width << "x" << height << " x " << layerCountU << " layers\n";
		return texture;
	}

	wgpu::Texture makeTexture(const std::filesystem::path& path, Device device, TextureView* pTextureView) {
		int width = 0, height = 0, channels = 0;
		const std::string filePath = path.string();
		const bool isHdr = stbi_is_hdr(filePath.c_str()) != 0;

		TextureDescriptor textureDesc;
		textureDesc.dimension = TextureDimension::_2D;
		textureDesc.sampleCount = 1;
		textureDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
		textureDesc.viewFormatCount = 0;
		textureDesc.viewFormats = nullptr;

		wgpu::Texture texture;

		if (isHdr) {
			// Keep the path robust and filterable across backends by converting HDR input to RGBA8.
			// Dynamic range is reduced, but this avoids half-float packing bugs/format mismatch artifacts.
			unsigned char* pixelData = stbi_load(filePath.c_str(), &width, &height, &channels, 4 /* force RGBA */);
			if (pixelData == nullptr) return nullptr;

			textureDesc.format = TextureFormat::RGBA8Unorm;
			textureDesc.size = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 };
			textureDesc.mipLevelCount = bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
			texture = device.createTexture(textureDesc);

			writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData);
			stbi_image_free(pixelData);
		} else {
			unsigned char* pixelData = stbi_load(filePath.c_str(), &width, &height, &channels, 4 /* force 4 channels */);
			if (pixelData == nullptr) return nullptr;

			textureDesc.format = TextureFormat::RGBA8Unorm;
			textureDesc.size = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 };
			textureDesc.mipLevelCount = bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
			texture = device.createTexture(textureDesc);

			writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData);
			stbi_image_free(pixelData);
		}

		if (pTextureView) {
			TextureViewDescriptor textureViewDesc;
			textureViewDesc.aspect = TextureAspect::All;
			textureViewDesc.baseArrayLayer = 0;
			textureViewDesc.arrayLayerCount = 1;
			textureViewDesc.baseMipLevel = 0;
			textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
			textureViewDesc.dimension = TextureViewDimension::_2D;
			textureViewDesc.format = textureDesc.format;
			*pTextureView = texture.createView(textureViewDesc);
		}

		return texture;
	}


	Texture loadTexture(wgpu::TextureView& textureView)
	{
		Texture texture;

		// Create a sampler
		SamplerDescriptor samplerDesc;
		samplerDesc.addressModeU = AddressMode::Repeat;
		samplerDesc.addressModeV = AddressMode::Repeat;
		samplerDesc.addressModeW = AddressMode::Repeat;
		samplerDesc.magFilter = FilterMode::Nearest; // or FilterMode::Linear
		samplerDesc.minFilter = FilterMode::Nearest; // or FilterMode::Linear
		samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 8.0f;
		samplerDesc.compare = CompareFunction::Undefined;
		samplerDesc.maxAnisotropy = 1;
		texture.sampler = device.createSampler(samplerDesc);

		// Assign the texture view passed in
		texture.textureView = textureView;

		if (!texture.textureView)
		{
			std::cerr << "Invalid TextureView passed to loadTextureFromView!\n";
		}
		else
		{
			std::cout << "Loaded TextureView successfully.\n";
		}

		return texture;
	}
}
