#pragma once

#include <filesystem>
#include <fstream>

#include "context.h"

namespace wgfx
{
	struct Texture
	{
		TextureView textureView;
		wgpu::Texture texture;
		Sampler sampler;

		int width;
		int height;

		Texture() = default;
	};

	Texture loadTextureSrc(int width, int height);
	Texture loadTextureDst(int width, int height);
	Texture loadTexture3D(int width, int height, int depth, wgpu::TextureFormat format = wgpu::TextureFormat::R32Uint);
	Texture loadTexture(const std::filesystem::path& path);
	Texture loadTextureArray(const std::vector<std::filesystem::path>& paths);
	/** Each layer is width*height*4 RGBA8 bytes; all layers same dimensions. Builds a 2D array texture with mipmaps.
	 *  If @p srgbEncoded, bytes are treated as sRGB (typical PNG/glTF base color); sampling returns linear RGB. */
	Texture loadTexture2DArrayFromRgba8(int width, int height, const std::vector<std::vector<unsigned char>>& layersRgba,
	    bool srgbEncoded = true);
	wgpu::Texture makeTexture(const std::filesystem::path& path, Device device, TextureView* pTextureView);
	Texture loadTexture(TextureView& texture);

}