#pragma once

#include <wgfx.h>

class Shader
{
public:
	Shader(const char* shader);

	void use();

	void setVar(wgfx::Uniform uniform);

private:
	wgfx::Pipeline pipeline;
	wgfx::RenderPass pass;
};