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

		renderPass.end();
		renderPass.release();
	}

	void RenderPass::setClear(WGPUColor color)
	{
		clearValue = color;
	}

	void RenderPass::draw(Pipeline* pipeline)
	{
		renderPass.setBindGroup(0, pipeline->uniforms.bindGroup, pipeline->uniforms.dynamicUniformCount, pipeline->uniforms.dynamicOffsets.data()); // or here
		// this "dynamic offset" value must be the issue, we need a dynamicOffset* instead. with a list of dynamic offsets
		renderPass.setPipeline(pipeline->pipeline);
		renderPass.setVertexBuffer(0, pipeline->vbos.current->buffer, 0, pipeline->vbos.current->buffer.getSize());
		renderPass.setIndexBuffer(pipeline->ibos.current->buffer, IndexFormat::Uint16, 0, pipeline->ibos.current->buffer.getSize());

		/*
		if (reset) //reset system for uniforms
		{
			for (auto uniform : pipeline->uniforms)d
			{
				uniform->quantity = 0;
			}
		}
		reset = false;
		*/
		//pipeline->uniforms.clear();
		
			//pipeline->index++; // incremenet vbo/ibo draw
		


		renderPass.drawIndexed(pipeline->ibos.current->indexCount, 1, 0, 0, 0);
	}

	void RenderPass::touch()
	{
		// i mean here we are touching the pass sooooo

		// Get the next surface texture view
		targetView = getNextSurfaceTextureView();
		if (!targetView) return;


		





		if (!updateMultiSampleView && multiSample)
		{
			multiSampleView = getMultiSampleView();
			if (!multiSampleView) return;
			updateMultiSampleView = true;
		}

		// Create a command encoder for the draw calls
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "Frame command encoder";
		encoder = device.createCommandEncoder();
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

#ifndef WEBGPU_BACKEND_WGPU
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

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
	}

	void RenderPass::prepareColor()
	{
		std::cout << "you working?\n";
		// this is sort of a getColorView(); method
		WGPUTextureDescriptor offscreenDesc = {};
		offscreenDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
		offscreenDesc.size = { (uint16_t)width, (uint16_t)height, 1 };
		offscreenDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
		offscreenDesc.sampleCount = 1; // Enable 4x MSAA
		offscreenDesc.mipLevelCount = 1;
		offscreenDesc.dimension = wgpu::TextureDimension::_2D;

		offscreenTexture = device.createTexture(offscreenDesc);
		offscreenView = offscreenTexture.createView();
		///
		if (!offscreenView) {
			std::cerr << "Failed to create offscreenView!" << std::endl;
		}
		else {
			std::cout << "offscreenView successfully created." << std::endl;
		}
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


		/*
		right i am going to create two, a multisampled and a viewTarget
		*/

		return targetView;
	}



	TextureView getMultiSampleView()
	{
		wgpu::TextureDescriptor multisampleTextureDesc = {};
		multisampleTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
		multisampleTextureDesc.size = { (uint16_t)width, (uint16_t)height, 1 };
		multisampleTextureDesc.format = format;
		multisampleTextureDesc.sampleCount = samples; // Enable 4x MSAA
		multisampleTextureDesc.mipLevelCount = 1;
		multisampleTextureDesc.dimension = wgpu::TextureDimension::_2D;

		wgpu::Texture multisampleTexture = device.createTexture(multisampleTextureDesc);
		wgpu::TextureView multisampleTextureView = multisampleTexture.createView();
		
		return multisampleTextureView;
	
	}

}