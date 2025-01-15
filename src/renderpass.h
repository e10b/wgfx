#pragma once

#include "pipeline.h"

namespace wgfx
{
	struct RenderPass
	{
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;



		RenderPass();

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);
		void touch();

	};
	inline TextureView multiSampleView = nullptr;
	inline TextureView targetView = nullptr;
	inline bool updateMultiSampleView = false;
	inline TextureFormat format = wgpu::TextureFormat::Undefined;

	TextureView getNextSurfaceTextureView();
	TextureView getMultiSampleView();
}