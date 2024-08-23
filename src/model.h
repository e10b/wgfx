#pragma once

#include <webgpu/webgpu.hpp>
#include "types.h"

using namespace wgpu;

class Model
{
public:
	Model()
	{
		createModel();
		initBuffers();
	}
private:
	Buffer vbo;//pointBuffer;
	Buffer ibo;
	u32 indexCount;
};