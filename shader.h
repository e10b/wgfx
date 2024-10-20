#pragma once

#include <wgfx.h>

class Shader
{
public:
	Shader(const char* shader);

	void use();

	void setVar(wgfx::Uniform uniform);

	void touch()
	{
		pass.touch();
	}

	wgfx::Pipeline pipeline;
private:
	wgfx::RenderPass pass;
};