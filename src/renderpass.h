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
		wgpu::Texture depthTexture = nullptr; // Store the texture so we can release it
		wgpu::TextureFormat format = depthTextureFormat;
		uint32_t fixedWidth = 0;
		uint32_t fixedHeight = 0;
		uint32_t textureWidth = 0;
		uint32_t textureHeight = 0;
		uint32_t textureSamples = 0;

		bool useDepth = false;
		DepthTexture(wgpu::TextureFormat format = depthTextureFormat) : format(format) { init(); }
		DepthTexture(uint32_t width, uint32_t height, wgpu::TextureFormat format = depthTextureFormat) : format(format), fixedWidth(width), fixedHeight(height) { init(); }
		~DepthTexture() {
			if (depthView) {
				depthView.release();
				depthView = nullptr;
			}
			if (depthTexture) {
				depthTexture.release();
				depthTexture = nullptr;
			}
		}

		void init()
		{
			if (depthView) {
				depthView.release();
				depthView = nullptr;
			}
			if (depthTexture) {
				depthTexture.release();
				depthTexture = nullptr;
			}

			textureWidth = fixedWidth ? fixedWidth : static_cast<uint32_t>(width);
			textureHeight = fixedHeight ? fixedHeight : static_cast<uint32_t>(height);
			textureSamples = samples;

			TextureDescriptor depthTextureDesc{};
			depthTextureDesc.dimension = TextureDimension::_2D;
			depthTextureDesc.format = format;
			depthTextureDesc.mipLevelCount = 1;
			depthTextureDesc.sampleCount = textureSamples;
			depthTextureDesc.size = { textureWidth, textureHeight, 1 };
			depthTextureDesc.usage = TextureUsage::RenderAttachment | TextureUsage::TextureBinding;
			depthTextureDesc.viewFormatCount = 1;
			depthTextureDesc.viewFormats = (WGPUTextureFormat*)&format;
			depthTexture = device.createTexture(depthTextureDesc);
			//std::cout << "Depth texture: " << depthTexture << std::endl;

			// Create the view of the depth texture manipulated by the rasterizer
			TextureViewDescriptor depthTextureViewDesc;
			depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
			depthTextureViewDesc.baseArrayLayer = 0;
			depthTextureViewDesc.arrayLayerCount = 1;
			depthTextureViewDesc.baseMipLevel = 0;
			depthTextureViewDesc.mipLevelCount = 1;
			depthTextureViewDesc.dimension = TextureViewDimension::_2D;
			depthTextureViewDesc.format = format;
			depthView = depthTexture.createView(depthTextureViewDesc);
			if (!depthView) { std::cerr << "Failed to create depth texture!\n"; }
			std::cout << "Depth texture: " << depthView << std::endl;
			//updateMultiSampleView = false;
			//resetDepth = true;
		}

		void ensureCurrentSize()
		{
			const uint32_t expectedWidth = fixedWidth ? fixedWidth : static_cast<uint32_t>(width);
			const uint32_t expectedHeight = fixedHeight ? fixedHeight : static_cast<uint32_t>(height);
			if (textureWidth != expectedWidth ||
				textureHeight != expectedHeight ||
				textureSamples != samples) {
				init();
			}
		}
	};

	struct ColorTexture
	{
	public:
		TextureView colorView = nullptr;
		wgpu::Texture colorTexture = nullptr; // Store the texture so we can release it
		bool isSurfaceTexture = false; // Flag to indicate if this is a surface texture
		wgpu::TextureFormat format = wgpu::TextureFormat::BGRA8UnormSrgb;

		ColorTexture(bool useSurface = true, wgpu::TextureFormat format = wgpu::TextureFormat::BGRA8UnormSrgb) : isSurfaceTexture(useSurface), format(format) {
			if (!useSurface) init();
		}
		~ColorTexture() {
			if (colorView) {
				colorView.release();
				colorView = nullptr;
			}
			if (colorTexture) {
				colorTexture.release();
				colorTexture = nullptr;
			}
		}

		void init()
		{
			std::cout << "Creating color texture!...\n";

			TextureDescriptor colorDesc = {};
			colorDesc.usage = wgpu::TextureUsage::RenderAttachment |
				wgpu::TextureUsage::TextureBinding |
				wgpu::TextureUsage::CopySrc;
			colorDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
			colorDesc.format = format;
			colorDesc.sampleCount = 1;
			colorDesc.mipLevelCount = 1;
			colorDesc.dimension = wgpu::TextureDimension::_2D;

			colorTexture = device.createTexture(colorDesc);
			colorView = colorTexture.createView();

			if (!colorView) { std::cerr << "Failed to create color texture!\n"; }
			std::cout << "Color texture: " << colorView << std::endl;
		}

	};

	TextureView getNextSurfaceTextureView();
	TextureView getMultiSampleView();

	inline void touch(ColorTexture* colorTex)
	{
		//// Only update surface textures
		//if (!colorTex.isSurfaceTexture) {
		//	std::cerr << "Warning: touch() called on non-surface ColorTexture" << std::endl;
		//	return;
		//}

		//// Release the previous color view if it exists
		if (colorTex->colorView) {
			colorTex->colorView.release();
			colorTex->colorView = nullptr;
		}

		colorTex->colorView = getNextSurfaceTextureView();
		if (!colorTex->colorView) return;

		
	}

	inline void start()
	{
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
		frameIndex++;
	}

	struct ComputePass
	{
	public:
		ComputePassEncoder computePass = nullptr;

		void draw(Compute* compute, size_t size)
		{
			computePass.setPipeline(compute->pipeline);
			compute->uniforms.clearForNewFrame();
			computePass.setBindGroup(0, compute->uniforms.bindGroup, compute->uniforms.dynamicUniformCount, compute->uniforms.dynamicOffsets.data());
			computePass.dispatchWorkgroups(static_cast<uint32_t>(size), 1, 1);
		}

		void drawXY(Compute* compute, uint32_t sizeX, uint32_t sizeY)
		{
			computePass.setPipeline(compute->pipeline);
			compute->uniforms.clearForNewFrame();
			computePass.setBindGroup(0, compute->uniforms.bindGroup, compute->uniforms.dynamicUniformCount, compute->uniforms.dynamicOffsets.data());
			computePass.dispatchWorkgroups(sizeX, sizeY, 1);
		}

		void drawXYZ(Compute* compute, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
		{
			computePass.setPipeline(compute->pipeline);
			compute->uniforms.clearForNewFrame();
			computePass.setBindGroup(0, compute->uniforms.bindGroup, compute->uniforms.dynamicUniformCount, compute->uniforms.dynamicOffsets.data());
			computePass.dispatchWorkgroups(sizeX, sizeY, sizeZ);
		}

		void end()
		{
			if (computePass)
			{
				computePass.end();
				computePass.release();
				computePass = nullptr;
			}
		}

		void prepare()
		{
			computePass = encoder.beginComputePass();
		}
		
	};

	struct RenderPass
	{
	public:
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;
		std::vector<ColorTexture*> colors;
		DepthTexture* depth = nullptr;
		bool shouldClear = true; // New member to control load operation
		//std::vector<RenderPassColorAttachment> colorAttachments; // Store attachments as member

		void addTarget(ColorTexture* color) { colors.push_back(color); }
		void addTarget(DepthTexture* depth) { depth->useDepth = true; this->depth = depth; }

		RenderPass();
		~RenderPass() {
			if (renderPass) {
				renderPass.end();
				renderPass.release();
				renderPass = nullptr;
			}
			//colorAttachments.clear();
		}

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);


		void prepare()
		{
			// Cleanup any existing render pass state

			// Create a new command encoder only if one doesn't exist
			/*if (!encoder) {
				CommandEncoderDescriptor encoderDesc = {};
				encoderDesc.label = "Render pass command encoder";
				encoder = device.createCommandEncoder(encoderDesc);
				if (!encoder) {
					std::cerr << "Failed to create command encoder!" << std::endl;
					return;
				}
			}*/
			//std::cout << "Preparing render pass with " << colors.size() << " color targets\n";
			// Prepare color attachments
			std::vector<RenderPassColorAttachment> colorAttachments;
			colorAttachments.reserve(colors.size());

			for (size_t i = 0; i < colors.size(); ++i) {
				if (!colors[i]->colorView) {
					std::cerr << "Invalid color view at index " << i << std::endl;
					return;
				}
				RenderPassColorAttachment attachment{};
				//std::cout << "HOW MANY???: " << colors.size() << "\n";
				if (wgfx::multiSample && colors[i]->isSurfaceTexture) {
					attachment.view = getMultiSampleView();
					attachment.resolveTarget = colors[i]->colorView;
				} else {
					attachment.view = colors[i]->colorView;
					attachment.resolveTarget = nullptr;
				}
				attachment.loadOp = shouldClear ? LoadOp::Clear : LoadOp::Load; // Use shouldClear flag
				//attachment.loadOp = LoadOp::Clear; // Always clear color attachment
				attachment.storeOp = StoreOp::Store;
				attachment.clearValue = clearValue;
#ifdef __EMSCRIPTEN__
				attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif
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
				depth->ensureCurrentSize();
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
}
