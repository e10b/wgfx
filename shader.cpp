#include "shader.h"

Shader::Shader(const char* shader)
{
	pipeline = wgfx::loadPipeline(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));
	pass.setClear({ 0.5, 0.5, 0.5, 1 });
}

void Shader::use()
{
	pass.draw(pipeline);
	pass.end();
}

void Shader::setVar(wgfx::Uniform uniform)
{
	pipeline.setUniform(uniform, true);
}

