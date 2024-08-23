#pragma once

#include "context.h"

#include <webgpu/webgpu.hpp>

class Shader
{
public:

	Shader(const char* shaderSource)
	{
		device = Context::getInstance().getDevice();
		createShaderModule(shaderSource);
		pipeline = makePipeline();
		//Context::getInstance().setShader(pipeline);
	
		/*make();
		renderPass.setPipeline(pipeline);
		renderPass.draw(3, 1, 0, 0);
		use();*/
	}

	void use()
	{
		make();
		renderPass.setPipeline(pipeline);
		renderPass.draw(3, 1, 0, 0);
		submit();
	}

	void make() //private
	{

		// Create the render pass that clears the screen with our color
		RenderPassDescriptor renderPassDesc = {};

		// The attachment part of the render pass descriptor describes the target texture of the pass
		RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = Context::getInstance().getView();
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Clear;
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;
		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;

		renderPass = Context::getInstance().getEncoder().beginRenderPass(renderPassDesc);

		// Select which render pipeline to use
				//renderPass.setPipeline(pipeline);
		// Draw 1 instance of a 3-vertices shape
				//renderPass.draw(3, 1, 0, 0);

			//renderPass.setPipeline(pipline2);

		//////////////////////////////////////////////////// me thinks a draw.
	//	renderPass.end();
	//	renderPass.release();
	}

	void submit()
	{
		renderPass.end();
		renderPass.release();
	}


private:

	RenderPassEncoder renderPass = nullptr;

	Device* device = nullptr;
	ShaderModule shaderModule = nullptr;
		RenderPipeline pipeline = nullptr;

	void createShaderModule(const char* shaderSource)
	{
		// Load the shader module
		ShaderModuleDescriptor shaderDesc;


		// We use the extension mechanism to specify the WGSL part of the shader module descriptor
		ShaderModuleWGSLDescriptor shaderCodeDesc;
		//ShaderModuleGLSLDescriptor; // nice!
// Set the chained struct's header
		shaderCodeDesc.chain.next = nullptr;
		shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
		// Connect the chain
		shaderDesc.nextInChain = &shaderCodeDesc.chain;
		shaderCodeDesc.code = shaderSource;
		shaderModule = device->createShaderModule(shaderDesc);
	}

	RenderPipeline makePipeline()
	{


		// Create the render pipeline
		RenderPipelineDescriptor pipelineDesc;

		// We do not use any vertex buffer for this first simplistic example
		pipelineDesc.vertex.bufferCount = 0;
		pipelineDesc.vertex.buffers = nullptr;

		// NB: We define the 'shaderModule' in the second part of this chapter.
		// Here we tell that the programmable vertex shader stage is described
		// by the function called 'vs_main' in that module.
		pipelineDesc.vertex.module = shaderModule;
		pipelineDesc.vertex.entryPoint = "vs_main";
		pipelineDesc.vertex.constantCount = 0;
		pipelineDesc.vertex.constants = nullptr;

		// Each sequence of 3 vertices is considered as a triangle
		pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;

		// We'll see later how to specify the order in which vertices should be
		// connected. When not specified, vertices are considered sequentially.
		pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;

		// The face orientation is defined by assuming that when looking
		// from the front of the face, its corner vertices are enumerated
		// in the counter-clockwise (CCW) order.
		pipelineDesc.primitive.frontFace = FrontFace::CCW;

		// But the face orientation does not matter much because we do not
		// cull (i.e. "hide") the faces pointing away from us (which is often
		// used for optimization).
		pipelineDesc.primitive.cullMode = CullMode::None;

		// We tell that the programmable fragment shader stage is described
		// by the function called 'fs_main' in the shader module.
		FragmentState fragmentState;
		fragmentState.module = shaderModule;
		fragmentState.entryPoint = "fs_main";
		fragmentState.constantCount = 0;
		fragmentState.constants = nullptr;

		BlendState blendState;
		blendState.color.srcFactor = BlendFactor::SrcAlpha;
		blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
		blendState.color.operation = BlendOperation::Add;
		blendState.alpha.srcFactor = BlendFactor::Zero;
		blendState.alpha.dstFactor = BlendFactor::One;
		blendState.alpha.operation = BlendOperation::Add;

		ColorTargetState colorTarget;
		colorTarget.format = Context::getInstance().getFormat();
		colorTarget.blend = &blendState;
		colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

		// We have only one target because our render pass has only one output color
		// attachment.
		fragmentState.targetCount = 1;
		fragmentState.targets = &colorTarget;
		pipelineDesc.fragment = &fragmentState;

		// We do not use stencil/depth testing for now
		pipelineDesc.depthStencil = nullptr;

		// Samples per pixel
		pipelineDesc.multisample.count = 1;

		// Default value for the mask, meaning "all bits on"
		pipelineDesc.multisample.mask = ~0u;

		// Default value as well (irrelevant for count = 1 anyways)
		pipelineDesc.multisample.alphaToCoverageEnabled = false;
		pipelineDesc.layout = nullptr;

		RenderPipeline pipeline = device->createRenderPipeline(pipelineDesc);

		// We no longer need to access the shader module
		shaderModule.release();

		return pipeline;
	}
};