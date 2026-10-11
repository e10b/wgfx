#pragma once

#include <context.h>

using namespace wgpu;
namespace wgfx
{
	struct FrameMetrics
	{
		uint64_t submitted = 0;
		uint64_t completed = 0;
		uint64_t totalCompletionNanoseconds = 0;
		uint64_t maximumCompletionNanoseconds = 0;
		uint32_t peakFramesInFlight = 0;
	};

	void init(Surface surface);

	void initSurface();

	Surface getSurface(SDL_Window* w);

	void frame();
	void configureFrameExecution(bool nonBlocking, bool collectMetrics,
		uint32_t maximumFramesInFlight = 3);
	FrameMetrics getFrameMetrics();
	void drainFrames();

	void initDepth();
	
}
