#pragma once

#include "pipeline.h"

namespace wgfx
{

	inline TextureView offscreenView = nullptr;
	inline wgpu::Texture offscreenTexture;
	inline TextureFormat format = wgpu::TextureFormat::Undefined;

	struct RenderPass
	{
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;



		RenderPass();

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);
		void touch();
		void scene(bool test);
		void post();


		void prepareColor();

	};
	inline TextureView multiSampleView = nullptr;
	inline TextureView targetView = nullptr;

	//testing here, hmmmmmm



	inline bool updateMultiSampleView = false;

	// Memory management utilities
	void cleanupStaticResources();

	TextureView getNextSurfaceTextureView();
	TextureView getMultiSampleView();
}