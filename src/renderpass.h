#pragma once

#include "pipeline.h"

namespace wgfx
{
	struct RenderPass
	{
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);
		void touch();

	};
	TextureView getNextSurfaceTextureView();
}