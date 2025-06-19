#include "renderpass.h"

namespace wgfx
{
	RenderPass::RenderPass()
	{
		// Get the next target texture view
	}

	void RenderPass::end()
	{
		//for (auto p : pipelines) // neccessary to loop through setters for vbos/ibos
		//{
		//	p->index = 0;
		//}

		if (renderPass) {
			renderPass.end();
			renderPass.release();
			renderPass = nullptr;
		}
		
		// Don't release targetView here - it's needed until frame submission
		// The surface texture view will be released automatically when the surface presents
	}

	void RenderPass::setClear(WGPUColor color)
	{
		clearValue = color;
	}
	void RenderPass::draw(Pipeline* pipeline)
	{
		// Reset uniforms at the start of each frame (only once per frame)
		if (reset) {
			pipeline->uniforms.clear();
			reset = false;
		}

		renderPass.setBindGroup(0, pipeline->uniforms.bindGroup, pipeline->uniforms.dynamicUniformCount, pipeline->uniforms.dynamicOffsets.data());
		renderPass.setPipeline(pipeline->pipeline);
		renderPass.setVertexBuffer(0, pipeline->vbos.current->buffer, 0, pipeline->vbos.current->buffer.getSize());
		renderPass.setIndexBuffer(pipeline->ibos.current->buffer, IndexFormat::Uint16, 0, pipeline->ibos.current->buffer.getSize());

		renderPass.drawIndexed(pipeline->ibos.current->indexCount, 1, 0, 0, 0);
	}
	void RenderPass::touch()
	{
		// Get the next surface texture view
		targetView = getNextSurfaceTextureView();
		if (!targetView) return;

		if (!updateMultiSampleView && multiSample)
		{
			multiSampleView = getMultiSampleView();
			if (!multiSampleView) return;
			updateMultiSampleView = true;
		}

		// Release previous command encoder if it exists
		if (encoder) {
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

	void RenderPass::post()
	{
		RenderPassDescriptor renderPassDesc{};

		RenderPassColorAttachment renderPassColorAttachment = {};
		//renderPassColorAttachment.view = targetView;  // No multisampling here for post-process
		renderPassColorAttachment.view = multiSample ? multiSampleView : targetView;
		renderPassColorAttachment.resolveTarget = multiSample ? targetView : nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Load;  // Keep content from scene pass
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = {};  // Not used with Load op

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		renderPassDesc.depthStencilAttachment = nullptr; // No depth/stencil for post-process
		renderPassDesc.timestampWrites = nullptr;

		renderPass = encoder.beginRenderPass(renderPassDesc);
	}
	void RenderPass::scene()
	{
		RenderPassDescriptor renderPassDesc{};

		RenderPassColorAttachment renderPassColorAttachment[2] = {};
		renderPassColorAttachment[0].view = multiSample ? multiSampleView : targetView;
		renderPassColorAttachment[0].resolveTarget = multiSample ? targetView : nullptr;
		renderPassColorAttachment[0].loadOp = LoadOp::Clear;
		renderPassColorAttachment[0].storeOp = StoreOp::Store;
		renderPassColorAttachment[0].clearValue = clearValue;
		/*
#ifndef WEBGPU_BACKEND_WGPU
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif
		*/
		renderPassColorAttachment[1].view = offscreenView;
		renderPassColorAttachment[1].resolveTarget = nullptr;
		renderPassColorAttachment[1].loadOp = LoadOp::Clear;
		renderPassColorAttachment[1].storeOp = StoreOp::Store;	
		renderPassColorAttachment[1].clearValue = clearValue;




		renderPassDesc.colorAttachmentCount = 2; /// two`	
		renderPassDesc.colorAttachments = renderPassColorAttachment;

		// Setup depth/stencil attachment
		RenderPassDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.view = depthTextureView;
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
		renderPassDesc.timestampWrites = nullptr;

		renderPass = encoder.beginRenderPass(renderPassDesc);
	}	void RenderPass::prepareColor()
	{
		std::cout << "you working?\n";
		
		// Use static variables to cache the offscreen texture and avoid recreating every call
		static wgpu::Texture cachedOffscreenTexture = nullptr;
		static wgpu::TextureView cachedOffscreenView = nullptr;
		static uint32_t cachedWidth = 0;
		static uint32_t cachedHeight = 0;
		
		// Only recreate if size changed or texture doesn't exist
		if (!cachedOffscreenTexture || cachedWidth != width || cachedHeight != height) {
			// Release previous resources
			if (cachedOffscreenView) {
				cachedOffscreenView.release();
				cachedOffscreenView = nullptr;
			}
			if (cachedOffscreenTexture) {
				cachedOffscreenTexture.release();
				cachedOffscreenTexture = nullptr;
			}
			
			// Create new texture
			WGPUTextureDescriptor offscreenDesc = {};
			offscreenDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
			offscreenDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
			offscreenDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
			offscreenDesc.sampleCount = 1;
			offscreenDesc.mipLevelCount = 1;
			offscreenDesc.dimension = wgpu::TextureDimension::_2D;

			cachedOffscreenTexture = device.createTexture(offscreenDesc);
			cachedOffscreenView = cachedOffscreenTexture.createView();
			
			// Update cached values
			cachedWidth = width;
			cachedHeight = height;
			
			if (!cachedOffscreenView) {
				std::cerr << "Failed to create offscreenView!" << std::endl;
			} else {
				std::cout << "offscreenView successfully created." << std::endl;
			}
		}
		
		// Update global references
		offscreenTexture = cachedOffscreenTexture;
		offscreenView = cachedOffscreenView;
	}

	TextureView getNextSurfaceTextureView()
	{
		// Get the surface texture
		SurfaceTexture surfaceTexture;
		surface.getCurrentTexture(&surfaceTexture);
		if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
			return nullptr;
		}
		wgpu::Texture texture = surfaceTexture.texture;
		format = texture.getFormat();
		
		// Create a view for this surface texture
		TextureViewDescriptor viewDescriptor;
		viewDescriptor.label = "Surface texture view";
		viewDescriptor.format = format;
		viewDescriptor.dimension = TextureViewDimension::_2D;
		viewDescriptor.baseMipLevel = 0;
		viewDescriptor.mipLevelCount = 1;
		viewDescriptor.baseArrayLayer = 0;
		viewDescriptor.arrayLayerCount = 1;
		viewDescriptor.aspect = TextureAspect::All;
		TextureView targetView = texture.createView(viewDescriptor);

		return targetView;
	}


	TextureView getMultiSampleView()
	{
		// Static variables to cache the multisample texture and avoid recreating every frame
		static wgpu::Texture multisampleTexture = nullptr;
		static wgpu::TextureView multisampleTextureView = nullptr;
		static uint32_t cachedWidth = 0;
		static uint32_t cachedHeight = 0;
		static TextureFormat cachedFormat = TextureFormat::Undefined;
		static uint32_t cachedSamples = 0;

		// Check if we need to recreate the texture (size, format, or sample count changed)
		if (!multisampleTexture || 
			cachedWidth != width || 
			cachedHeight != height || 
			cachedFormat != format || 
			cachedSamples != samples) {
			
			// Release previous texture if it exists
			if (multisampleTexture) {
				multisampleTextureView.release();
				multisampleTexture.release();
			}

			wgpu::TextureDescriptor multisampleTextureDesc = {};
			multisampleTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
			multisampleTextureDesc.size = { (uint16_t)width, (uint16_t)height, 1 };
			multisampleTextureDesc.format = format;
			multisampleTextureDesc.sampleCount = samples; // Enable 4x MSAA
			multisampleTextureDesc.mipLevelCount = 1;
			multisampleTextureDesc.dimension = wgpu::TextureDimension::_2D;

			multisampleTexture = device.createTexture(multisampleTextureDesc);
			multisampleTextureView = multisampleTexture.createView();
			
			// Update cached values
			cachedWidth = width;
			cachedHeight = height;
			cachedFormat = format;
			cachedSamples = samples;
		}
		
		return multisampleTextureView;
	}

	void cleanupStaticResources()
	{
		// Clean up static cached resources
		static wgpu::Texture* multisampleTexture = nullptr;
		static wgpu::TextureView* multisampleTextureView = nullptr;
		static wgpu::Texture* cachedOffscreenTexture = nullptr;
		static wgpu::TextureView* cachedOffscreenView = nullptr;
		static wgpu::Texture* previousDepthTexture = nullptr;
		
		// Get static variables from each function and clean them up
		// This would normally require refactoring to make these accessible
		// For now, we'll add cleanup in the destructor or app exit
	}
}