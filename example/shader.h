#pragma once

#include <wgfx.h>

class Shader
{
public:
	Shader() = default;

	Shader(const char* shader)
	{
		pipeline = wgfx::loadPipeline(wgfx::loadFromFile((std::string(RESOURCE_DIR) + "/" + shader).c_str()));
		renderPass.setClear({ 0.0375, 0.0375, 0.0375, 1 });
	}

	void use()
	{
		renderPass.draw(pipeline);
	}

	void setUniform(int index)
	{
		wgfx::Uniform* uniform = wgfx::createUniform(index, sizeof(glm::mat4), 1.0f);
		pipeline.setUniform(uniform, true);
		uniforms.push_back(uniform);
	}

	void setUniform(int index, glm::mat4& value)
	{
		wgfx::Uniform* uniform = wgfx::createUniform(index, sizeof(glm::mat4), glm::value_ptr(value));
		pipeline.setUniform(uniform, true);
		uniforms.push_back(uniform);
	}

	void setTexture(int index, wgfx::Texture texture)
	{
		wgfx::Uniform* uniform = wgfx::createTexture(index, texture);
		pipeline.setTexture(uniform);
		uniforms.push_back(uniform);
	}

	void setSampler(int index, wgfx::Texture texture)
	{
		wgfx::Uniform* uniform = wgfx::createSampler(index, texture);
		pipeline.setSampler(uniform);
		uniforms.push_back(uniform);
	}

	void updateUniform(int index, glm::mat4& value)
	{
		pipeline.updateUniform(uniforms.at(index), glm::value_ptr(value));
	}



	std::vector<wgfx::Uniform*> uniforms;

	wgfx::Pipeline pipeline;
	wgfx::RenderPass renderPass;
private:
};