#include "surface.h"
#include "renderpass.h"
#include <cstring>
#include <iostream>
namespace wgfx
{
	namespace
	{
		void requestDevice()
		{
			std::cout << "Requesting device..." << std::endl;

			SupportedLimits adapterSupportedLimits;
			adapter.getLimits(&adapterSupportedLimits);
			std::cout << "Adapter supported maxStorageBufferBindingSize: "
			          << adapterSupportedLimits.limits.maxStorageBufferBindingSize << " bytes" << std::endl;

			RequiredLimits requiredLimits = {};
			memset(&requiredLimits.limits, 0xFF, sizeof(WGPULimits));
			requiredLimits.limits.maxStorageBufferBindingSize = 1073741824;
			requiredLimits.limits.maxBufferSize = 1073741824;
			requiredLimits.limits.maxComputeWorkgroupStorageSize = 32768;
			requiredLimits.limits.maxStorageBuffersPerShaderStage = 8;

			DeviceDescriptor deviceDesc = {};
			deviceDesc.label = "My Device";
			deviceDesc.requiredFeatureCount = 0;
#ifdef __EMSCRIPTEN__
			deviceDesc.requiredLimits = nullptr;
#else
			deviceDesc.requiredLimits = &requiredLimits;
#endif
			deviceDesc.defaultQueue.nextInChain = nullptr;
			deviceDesc.defaultQueue.label = "The default queue";
			deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void*) {
				std::cout << "Device lost: reason " << reason;
				if (message) std::cout << " (" << message << ")";
				std::cout << std::endl;
			};
			device = adapter.requestDevice(deviceDesc);
			std::cout << "Got device: " << device << std::endl;

			SupportedLimits deviceSupportedLimits;
			device.getLimits(&deviceSupportedLimits);
			deviceLimits = deviceSupportedLimits.limits;

			std::cout << "Device maxStorageBufferBindingSize: " << deviceLimits.maxStorageBufferBindingSize << " bytes ("
			          << (deviceLimits.maxStorageBufferBindingSize / (1024.0 * 1024.0 * 1024.0)) << " GB)" << std::endl;

			uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
				std::cout << "Uncaptured device error: type " << type;
				if (message) std::cout << " (" << message << ")";
				std::cout << std::endl;
			});

			queue = device.getQueue();
		}
	}

	void init()
	{
		instance = wgpuCreateInstance(nullptr);
		std::cout << "Requesting adapter..." << std::endl;

		RequestAdapterOptions adapterOpts = {};
		adapter = instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		instance.release();
		requestDevice();
	}

	void init(Surface surface)
	{
		// INIT DEVICE
		RequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		adapter = instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		instance.release();

		requestDevice();

#ifdef WGFX_ENABLE_SDL
		initSurface();
#endif
	}

#ifdef WGFX_ENABLE_SDL
	void initSurface()
	{
		SDL_GetWindowSize(window, &width, &height);
		if (width == 0) width = 1;
		if (height == 0) height = 1;

		std::cout << "Creating swapchain (size: " << width << "x" << height << ")...\n";
		// Configure the surface
		SurfaceConfiguration config = {};

		// Configuration of the textures created for the underlying swap chain
		config.width = width;
		config.height = height;
		config.usage = TextureUsage::RenderAttachment;
		surfaceFormat = surface.getPreferredFormat(adapter);
		config.format = surfaceFormat;

		// And we do not need any particular view format:
		config.viewFormatCount = 0;
		config.viewFormats = nullptr;
		config.device = device;
		config.presentMode = PresentMode::Fifo; // vsync on for web compatibility
		config.alphaMode = CompositeAlphaMode::Opaque;

		surface.configure(config);

		// Release the adapter only after it has been fully utilized
		//adapter.release(); << maintain the adapter for reinit surface 
	}

	Surface getSurface(SDL_Window* w)
	{
		window = w;
		instance = wgpuCreateInstance(nullptr);
		std::cout << "Requesting adapter..." << std::endl;
		return surface = SDL_GetWGPUSurface(instance, w);
	}
#endif

	void frame()
	{
		if (!encoder) {
			std::cerr << "wgfx::frame: no command encoder (call wgfx::start() after touch and before encoding passes).\n";
			return;
		}
		reset = true;
		// nope you need a separate end
		//program.framebuffers.at(0)->renderPass.end();
		//program.framebuffers.at(0)->renderPass.release();
		//renderPass.
												//renderPass.renderPass.end();
												//renderPass.renderPass.release();
// Finally encode and submit the render pass
		CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();
		encoder = nullptr; // Clear the reference

		//std::cout << "Submitting command..." << std::endl;
		queue.submit(1, &command);
		command.release();
		//std::cout << "Command submitted." << std::endl;

		// At the end of the frame
		//targetView.release();
#ifndef __EMSCRIPTEN__
		if (surface) surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
#elif defined(WEBGPU_BACKEND_WGPU)
		//device.tick();
		//device.poll(false);
#endif
	}

	//void initDepth()
	//{
	//	SDL_GetWindowSize(window, &width, &height);

	//	// Create the depth texture
	//	TextureDescriptor depthTextureDesc;
	//	depthTextureDesc.dimension = TextureDimension::_2D;
	//	depthTextureDesc.format = depthTextureFormat;
	//	depthTextureDesc.mipLevelCount = 1;
	//	depthTextureDesc.sampleCount = samples;
	//	depthTextureDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
	//	depthTextureDesc.usage = TextureUsage::RenderAttachment;
	//	depthTextureDesc.viewFormatCount = 1;
	//	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	//	wgpu::Texture depthTexture = device.createTexture(depthTextureDesc);
	//	std::cout << "Depth texture: " << depthTexture << std::endl;

	//	// Create the view of the depth texture manipulated by the rasterizer
	//	TextureViewDescriptor depthTextureViewDesc;
	//	depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
	//	depthTextureViewDesc.baseArrayLayer = 0;
	//	depthTextureViewDesc.arrayLayerCount = 1;
	//	depthTextureViewDesc.baseMipLevel = 0;
	//	depthTextureViewDesc.mipLevelCount = 1;
	//	depthTextureViewDesc.dimension = TextureViewDimension::_2D;
	//	depthTextureViewDesc.format = depthTextureFormat;
	//	depthTextureView = depthTexture.createView(depthTextureViewDesc);
	//	std::cout << "Depth texture view: " << depthTextureView << std::endl;
	//	updateMultiSampleView = false;
	//}
}
