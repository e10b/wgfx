#pragma once

#include <filesystem>
#include <fstream>

#include "context.h"

namespace wgfx
{
	struct Texture
	{
		TextureView textureView;
		Sampler sampler;

		Texture() = default;
	};

	Texture loadTexture(const std::filesystem::path& path);
	wgpu::Texture makeTexture(const std::filesystem::path& path, Device device, TextureView* pTextureView);
	
	Texture loadTexture(TextureView& texture);


}