#include "surface.h"
#include "renderpass.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>
namespace wgfx
{
	namespace
	{
		using FrameClock = std::chrono::steady_clock;
		struct PendingFrame
		{
			FrameClock::time_point submittedAt;
			std::unique_ptr<QueueWorkDoneCallback> callback;
			bool finished = false;
		};
		bool nonBlockingFrames = false;
		bool collectFrameMetrics = false;
		uint32_t maximumFramesInFlight = 3;
		FrameMetrics frameMetrics;
		std::vector<std::shared_ptr<PendingFrame>> pendingFrames;

		void removeCompletedFrames()
		{
			pendingFrames.erase(std::remove_if(pendingFrames.begin(), pendingFrames.end(),
				[](const std::shared_ptr<PendingFrame>& frame) { return frame->finished; }),
				pendingFrames.end());
		}

		void trackSubmittedFrame()
		{
			if (!collectFrameMetrics && !nonBlockingFrames)
				return;
			auto frame = std::make_shared<PendingFrame>();
			frame->submittedAt = FrameClock::now();
			if (collectFrameMetrics)
				++frameMetrics.submitted;
			pendingFrames.push_back(frame);
			if (collectFrameMetrics)
				frameMetrics.peakFramesInFlight = std::max<uint32_t>(
					frameMetrics.peakFramesInFlight,
					static_cast<uint32_t>(pendingFrames.size()));
			std::weak_ptr<PendingFrame> weakFrame = frame;
			frame->callback = queue.onSubmittedWorkDone(
				[weakFrame](QueueWorkDoneStatus status)
				{
					auto completedFrame = weakFrame.lock();
					if (!completedFrame)
						return;
					if (status == QueueWorkDoneStatus::Success && collectFrameMetrics)
					{
						const uint64_t elapsed = std::chrono::duration_cast<
							std::chrono::nanoseconds>(FrameClock::now() -
							completedFrame->submittedAt).count();
						++frameMetrics.completed;
						frameMetrics.totalCompletionNanoseconds += elapsed;
						frameMetrics.maximumCompletionNanoseconds =
							std::max(frameMetrics.maximumCompletionNanoseconds, elapsed);
					}
					completedFrame->finished = true;
				});
		}
	}

	void configureFrameExecution(bool nonBlocking, bool collectMetrics,
		uint32_t maxFramesInFlight)
	{
		nonBlockingFrames = nonBlocking;
		collectFrameMetrics = collectMetrics;
		maximumFramesInFlight = std::max(1u, maxFramesInFlight);
		frameMetrics = {};
		pendingFrames.clear();
	}

	FrameMetrics getFrameMetrics() { return frameMetrics; }

	void drainFrames()
	{
#if defined(WEBGPU_BACKEND_WGPU)
		while (std::any_of(pendingFrames.begin(), pendingFrames.end(),
			[](const std::shared_ptr<PendingFrame>& frame) { return !frame->finished; }))
			device.poll(true);
#elif defined(WEBGPU_BACKEND_DAWN)
		device.tick();
#endif
		removeCompletedFrames();
	}

	void init(Surface surface)
	{
		// INIT DEVICE
		RequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		adapter = instance.requestAdapter(adapterOpts);
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

		// Get device limits
		SupportedLimits deviceSupportedLimits;
		device.getLimits(&deviceSupportedLimits);
		deviceLimits = deviceSupportedLimits.limits;

		uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
			std::cout << "Uncaptured device error: type " << type;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
			});

		queue = device.getQueue();

		initSurface();
	}

	void initSurface()
	{
		//int w, h;
		SDL_GetWindowSize(window, &width, &height);

		std::cout << "Creating swapchain...\n";
		// Configure the surface
		SurfaceConfiguration config = {};

		// Configuration of the textures created for the underlying swap chain
		config.width = width;
		config.height = height;
		config.usage = TextureUsage::RenderAttachment;
		surfaceFormat = surface.getPreferredFormat(adapter);
		config.format = surfaceFormat;//TextureFormat::BGRA8Unorm; //surfaceFormat

		// And we do not need any particular view format:
		config.viewFormatCount = 0;
		config.viewFormats = nullptr;
		config.device = device;
		config.presentMode = PresentMode::Immediate; // vysnc off
		config.alphaMode = CompositeAlphaMode::Auto;

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
	
	
	void frame()
	{
		reset = true;

		if (resetDepth) { resetDepth = false; }

		// Finally encode and submit the render pass
		CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();
		encoder = nullptr; // Clear the reference

		//std::cout << "Submitting command..." << std::endl;
		queue.submit(1, &command);
		trackSubmittedFrame();
		command.release();
		//std::cout << "Command submitted." << std::endl;

		// Present the surface
#ifndef __EMSCRIPTEN__
		surface.present();
#endif

		// Release the target view after presenting - this is critical for memory management
		if (targetView) {
			targetView.release();
			targetView = nullptr;
		}
		// Force WebGPU to clean up resources more aggressively
#if defined(WEBGPU_BACKEND_DAWN)
		device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
		device.poll(false);
		if (!nonBlockingFrames)
			device.poll(true);
		else if (pendingFrames.size() >= maximumFramesInFlight)
		{
			removeCompletedFrames();
			if (pendingFrames.size() >= maximumFramesInFlight)
				device.poll(true);
		}
#endif
		removeCompletedFrames();
	}

	void initDepth()
	{
		SDL_GetWindowSize(window, &width, &height);

		// Release previous depth texture and view if they exist
		/*static wgpu::Texture previousDepthTexture = nullptr;
		if (depthTextureView) {
			depthTextureView.release();
			depthTextureView = nullptr;
		}
		if (previousDepthTexture) {
			previousDepthTexture.release();
			previousDepthTexture = nullptr;
		}*/

		// Create the depth texture
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
		std::cout << "Depth texture: " << depthTexture << std::endl;

		// Create the view of the depth texture manipulated by the rasterizer
		TextureViewDescriptor depthTextureViewDesc;
		depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
		depthTextureViewDesc.baseArrayLayer = 0;
		depthTextureViewDesc.arrayLayerCount = 1;
		depthTextureViewDesc.baseMipLevel = 0;
		depthTextureViewDesc.mipLevelCount = 1;
		depthTextureViewDesc.dimension = TextureViewDimension::_2D;
		depthTextureViewDesc.format = depthTextureFormat;
		depthTextureView = depthTexture.createView(depthTextureViewDesc);
		std::cout << "Depth texture view: " << depthTextureView << std::endl;
		updateMultiSampleView = false;
		resetDepth = true;
		// Store reference for cleanup in next call
		//previousDepthTexture = depthTexture;
	}
}
