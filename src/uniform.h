#pragma once

#include "texture.h"

namespace wgfx
{
	struct Uniform
	{
		Buffer buffer;
		BindGroupEntry binding;
		int index;
		size_t scale;

		uint32_t stride;
		int offset;
		int quantity = 0; // uncrease when hmm, 


		Uniform() = default;

	};

	Uniform* createUniform(int i, size_t size, float data);
		Uniform createUniform(int i, size_t size, const float* array);
	Uniform* createTexture(int i, Texture texture);
	Uniform* createSampler(int i, Texture texture);
}