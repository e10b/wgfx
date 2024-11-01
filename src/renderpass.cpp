#include "renderpass.h"

namespace wgfx
{
	void RenderPass::end()
	{
		for (auto p : pipelines) // neccessary to loop through setters for vbos/ibos
		{
			p->index = 0;
		}

		renderPass.end();
		renderPass.release();
	}

	void RenderPass::setClear(WGPUColor color)
	{
		clearValue = color;
	}

	void RenderPass::draw(Pipeline* pipeline)
	{
		renderPass.setBindGroup(0, pipeline->bindGroup, pipeline->dynamicUniformCount, pipeline->dynamicOffsets.data()); // or here
		// this "dynamic offset" value must be the issue, we need a dynamicOffset* instead. with a list of dynamic offsets
		renderPass.setPipeline(pipeline->pipeline);
		renderPass.setVertexBuffer(0, pipeline->vbos.current->buffer, 0, pipeline->vbos.current->buffer.getSize());
		renderPass.setIndexBuffer(pipeline->ibos.current->buffer, IndexFormat::Uint16, 0, pipeline->ibos.current->buffer.getSize());

		if (reset) //reset system for uniforms
		{
			for (auto uniform : pipeline->uniforms)
			{
				uniform->quantity = 0;
			}
		}
		reset = false;
		
		pipeline->index++; // incremenet vbo/ibo draw
		


		renderPass.drawIndexed(pipeline->ibos.current->indexCount, 1, 0, 0, 0);
	}

	void RenderPass::touch()
	{
		// Get the next target texture view
		targetView = getNextSurfaceTextureView();
		if (!targetView) return;

		// Create a command encoder for the draw call
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "My command encoder";
		//encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
		encoder = device.createCommandEncoder();

		// Create the render pass that clears the screen with our color
		RenderPassDescriptor renderPassDesc{};

		// The attachment part of the render pass descriptor describes the target texture of the pass
		RenderPassColorAttachment renderPassColorAttachment = {};

		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Clear;
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = clearValue;
#ifndef WEBGPU_BACKEND_WGPU
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		// We now add a depth/stencil attachment:
		RenderPassDepthStencilAttachment depthStencilAttachment;
		// The view of the depth texture
		depthStencilAttachment.view = depthTextureView;

		// The initial value of the depth buffer, meaning "far"
		depthStencilAttachment.depthClearValue = 1.0f;
		// Operation settings comparable to the color attachment
		depthStencilAttachment.depthLoadOp = LoadOp::Clear;
		depthStencilAttachment.depthStoreOp = StoreOp::Store;
		// we could turn off writing to the depth buffer globally here
		depthStencilAttachment.depthReadOnly = false;

		// Stencil setup, mandatory but unused
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
		//renderPassDesc.depthStencilAttachment = nullptr;

		renderPassDesc.timestampWrites = nullptr;
		//renderPass = encoder.beginRenderPass(renderPassDesc);

		renderPass = encoder.beginRenderPass(renderPassDesc);

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

		// Create a view for this surface texture
		TextureViewDescriptor viewDescriptor;
		viewDescriptor.label = "Surface texture view";
		viewDescriptor.format = texture.getFormat();
		viewDescriptor.dimension = TextureViewDimension::_2D;
		viewDescriptor.baseMipLevel = 0;
		viewDescriptor.mipLevelCount = 1;
		viewDescriptor.baseArrayLayer = 0;
		viewDescriptor.arrayLayerCount = 1;
		viewDescriptor.aspect = TextureAspect::All;
		TextureView targetView = texture.createView(viewDescriptor);

		return targetView;
	}

}