#pragma once

#include "pipeline.h"

namespace wgfx
{

	inline TextureView offscreenView = nullptr;
	inline wgpu::Texture offscreenTexture;
	inline TextureFormat format = wgpu::TextureFormat::Undefined;

	struct DepthTexture
	{
		TextureView depthView = nullptr;

		bool useDepth = false;
		DepthTexture() { init(); }

		void init()
		{
			TextureDescriptor depthTextureDesc;
			depthTextureDesc.dimension = TextureDimension::_2D;
			depthTextureDesc.format = depthTextureFormat;
			depthTextureDesc.mipLevelCount = 1;
			depthTextureDesc.sampleCount = samples;
			depthTextureDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
			depthTextureDesc.usage = TextureUsage::RenderAttachment | TextureUsage::TextureBinding;
			depthTextureDesc.viewFormatCount = 1;
			depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
			wgpu::Texture depthTexture = device.createTexture(depthTextureDesc);
			//std::cout << "Depth texture: " << depthTexture << std::endl;

			// Create the view of the depth texture manipulated by the rasterizer
			TextureViewDescriptor depthTextureViewDesc;
			depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
			depthTextureViewDesc.baseArrayLayer = 0;
			depthTextureViewDesc.arrayLayerCount = 1;
			depthTextureViewDesc.baseMipLevel = 0;
			depthTextureViewDesc.mipLevelCount = 1;
			depthTextureViewDesc.dimension = TextureViewDimension::_2D;
			depthTextureViewDesc.format = depthTextureFormat;
			depthView = depthTexture.createView(depthTextureViewDesc);
			if (!depthView) { std::cerr << "Failed to create depth texture!\n"; }
			std::cout << "Depth texture: " << depthView << std::endl;
			//updateMultiSampleView = false;
			//resetDepth = true;
		}
	};

	struct ColorTexture
	{
	public:
		TextureView colorView = nullptr;

		ColorTexture() { init(); }

		void init()
		{
			std::cout << "Creating color texture!...\n";

			TextureDescriptor colorDesc = {};
			colorDesc.usage = wgpu::TextureUsage::RenderAttachment |
				wgpu::TextureUsage::TextureBinding |
				wgpu::TextureUsage::CopySrc;
			colorDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
			colorDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
			colorDesc.sampleCount = 1;
			colorDesc.mipLevelCount = 1;
			colorDesc.dimension = wgpu::TextureDimension::_2D;

			wgpu::Texture colorTexture = device.createTexture(colorDesc);
			colorView = colorTexture.createView();

			if (!colorView) { std::cerr << "Failed to create color texture!\n"; }
			std::cout << "Color texture: " << colorView << std::endl;
		}

	};

	TextureView getNextSurfaceTextureView();
	inline void touch(ColorTexture& colorTex)
	{
		colorTex.colorView = getNextSurfaceTextureView();
		if (!colorTex.colorView) return;

		if (encoder)
		{
			encoder.release();
			encoder = nullptr;
		}

		// Create a command encoder for the draw calls
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "Frame command encoder";
		encoder = device.createCommandEncoder();

		// Reset the global reset flag to ensure uniforms get reset
		reset = true;
	}

	struct RenderPass
	{
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;
		std::vector<ColorTexture*> colors;
		DepthTexture* depth = nullptr;

		void addTarget(ColorTexture& color) { colors.push_back(&color); }
		void addTarget(DepthTexture& depth) { depth.useDepth = true; this->depth = &depth; }

		RenderPass();

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);


		void prepare()
		{
			// Cleanup any existing render pass state

			// Create a new command encoder only if one doesn't exist
			if (!encoder) {
				CommandEncoderDescriptor encoderDesc = {};
				encoderDesc.label = "Render pass command encoder";
				encoder = device.createCommandEncoder(encoderDesc);
				if (!encoder) {
					std::cerr << "Failed to create command encoder!" << std::endl;
					return;
				}
			}
			std::cout << "Preparing render pass with " << colors.size() << " color targets\n";
			// Prepare color attachments
			std::vector<RenderPassColorAttachment> colorAttachments;
			colorAttachments.reserve(colors.size());

			for (size_t i = 0; i < colors.size(); ++i) {
				if (!colors[i]->colorView) {
					std::cerr << "Invalid color view at index " << i << std::endl;
					return;
				}
				RenderPassColorAttachment attachment{};
				attachment.view = colors[i]->colorView;
				attachment.resolveTarget = nullptr;
				attachment.loadOp = LoadOp::Clear;
				attachment.storeOp = StoreOp::Store;
				attachment.clearValue = clearValue;
				colorAttachments.push_back(attachment);
			}

			// Create render pass descriptor
			RenderPassDescriptor renderPassDesc{};
			renderPassDesc.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
			renderPassDesc.colorAttachments = colorAttachments.data();
			renderPassDesc.timestampWrites = nullptr;

			// Setup depth-stencil if needed
			RenderPassDepthStencilAttachment depthStencilAttachment{};
			if (depth && depth->depthView && depth->useDepth) {
				depthStencilAttachment.view = depth->depthView;
				depthStencilAttachment.depthClearValue = 1.0f;
				depthStencilAttachment.depthLoadOp = LoadOp::Clear;
				depthStencilAttachment.depthStoreOp = StoreOp::Store;
				depthStencilAttachment.depthReadOnly = false;
				depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
				depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
				depthStencilAttachment.stencilStoreOp = StoreOp::Store;
#else
				depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
				depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
#endif
				depthStencilAttachment.stencilReadOnly = true;
				renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
			}
			else
			{
				renderPassDesc.depthStencilAttachment = nullptr;
			}

			// Begin the render pass
			renderPass = encoder.beginRenderPass(renderPassDesc);
			if (!renderPass) {
				std::cerr << "Failed to begin render pass!" << std::endl;
				return;
			}

		}

		//void touch();

		//void scene(bool test);
		//void post();


		//void prepareColor();

	};
	inline TextureView multiSampleView = nullptr;
	inline TextureView targetView = nullptr;

	//testing here, hmmmmmm



	inline bool updateMultiSampleView = false;

	// Memory management utilities
	void cleanupStaticResources();

	TextureView getMultiSampleView();
}