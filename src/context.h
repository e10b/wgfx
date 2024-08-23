#pragma once

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL.h>

#include <iostream>
#include <cassert>
#include <vector>

using namespace wgpu;

const char* shaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

class Context
{
public:
	static Context& getInstance()
	{
		static Context inst;
		return inst;
	}

	Device* getDevice()
	{
		return &device;
	}

	TextureFormat getFormat()
	{
		return surfaceFormat;
	}

	void setShader(RenderPipeline p)
	{
		pipeline = p;
	}

	Context() {}

	void init()
	{
		SDL_SetMainReady();
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cerr << "Could not initialize SDL! Error: " << SDL_GetError() << std::endl;
			//return 1;
		}

		int windowFlags = 0;//SDL_WINDOW_RESIZABLE;
		SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);


		Instance instance = wgpuCreateInstance(nullptr);

		//surface = glfwGetWGPUSurface(instance, window);
		surface = SDL_GetWGPUSurface(instance, window);

		std::cout << "Requesting adapter..." << std::endl;
		surface = SDL_GetWGPUSurface(instance, window);
		//surface = glfwGetWGPUSurface(instance, window);
		RequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		Adapter adapter = instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		instance.release();

		std::cout << "Requesting device..." << std::endl;
		DeviceDescriptor deviceDesc = {};
		deviceDesc.label = "My Device";
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredLimits = nullptr;
		deviceDesc.defaultQueue.nextInChain = nullptr;
		deviceDesc.defaultQueue.label = "The default queue";
		deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
			std::cout << "Device lost: reason " << reason;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
			};
		device = adapter.requestDevice(deviceDesc);
		std::cout << "Got device: " << device << std::endl;

		uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
			std::cout << "Uncaptured device error: type " << type;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
			});

		queue = device.getQueue();

		// Configure the surface
		SurfaceConfiguration config = {};

		// Configuration of the textures created for the underlying swap chain
		config.width = 640;
		config.height = 480;
		config.usage = TextureUsage::RenderAttachment;
		surfaceFormat = surface.getPreferredFormat(adapter);
		config.format = surfaceFormat;

		// And we do not need any particular view format:
		config.viewFormatCount = 0;
		config.viewFormats = nullptr;
		config.device = device;
		config.presentMode = PresentMode::Fifo;
		config.alphaMode = CompositeAlphaMode::Auto;

		surface.configure(config);

		// Release the adapter only after it has been fully utilized
		adapter.release();

		//initPipeline(); move to shader class
	}

	~Context()
	{
		pipeline.release();
		surface.unconfigure();
		queue.release();
		surface.release();
		device.release();
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void update()
	{
		//glfwPollEvents();
		pollEvents();

		// Get the next target texture view
		TextureView targetView = getNextSurfaceTextureView();
		if (!targetView) return;

		// Create a command encoder for the draw call
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "My command encoder";
		CommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

		// Create the render pass that clears the screen with our color
		RenderPassDescriptor renderPassDesc = {};

		// The attachment part of the render pass descriptor describes the target texture of the pass
		RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = targetView;
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

		RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

		// Select which render pipeline to use
			renderPass.setPipeline(pipeline);
		// Draw 1 instance of a 3-vertices shape
			renderPass.draw(3, 1, 0, 0);

		//////////////////////////////////////////////////// me thinks a draw.
		renderPass.end();
		renderPass.release();

		// Finally encode and submit the render pass
		CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();

		std::cout << "Submitting command..." << std::endl;
		queue.submit(1, &command);
		command.release();
		std::cout << "Command submitted." << std::endl;

		// At the end of the frame
		targetView.release();
#ifndef __EMSCRIPTEN__
		surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
#elif defined(WEBGPU_BACKEND_WGPU)
		//device.tick();
		//device.poll(false);
		//device.ti
#endif
	}

	bool isRunning()
	{
		return !shouldClose;
	}

	void pollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				shouldClose = true;
				break;

			default:
				break;
			}
		}
	}

	bool shouldClose = false;

private:
	TextureView getNextSurfaceTextureView()
	{
		// Get the surface texture
		SurfaceTexture surfaceTexture;
		surface.getCurrentTexture(&surfaceTexture);
		if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
			return nullptr;
		}
		Texture texture = surfaceTexture.texture;

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

	void initPipeline() 
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
		ShaderModule shaderModule = device.createShaderModule(shaderDesc);









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
		colorTarget.format = surfaceFormat;
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

		pipeline = device.createRenderPipeline(pipelineDesc);

		// We no longer need to access the shader module
		shaderModule.release();
	
	}


private:
	SDL_Window* window = nullptr;
	Device device = nullptr;
	Queue queue = nullptr;
	Surface surface = nullptr;
	std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;
	TextureFormat surfaceFormat = TextureFormat::Undefined;
	RenderPipeline pipeline = nullptr;
};