#pragma once

#include <wgfx.h>

class Shader
{
public:
	Shader(const char* shader)
	{
		pipeline = wgfx::loadPipeline(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));
		renderPass.setClear({ 0.0375, 0.0375, 0.0375, 1 });
	}

	void use()
	{
		renderPass.draw(pipeline);
	}

	wgfx::Pipeline pipeline;
	wgfx::RenderPass renderPass;
private:
};