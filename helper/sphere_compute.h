#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "constants.h"
#include "wgfx.h"

class SphereCompute
{
public:
	static SphereCompute& Instance()
	{
		static SphereCompute instance;
		return instance;
	}

	wgfx::Texture outputTexture;

	void render(const glm::vec3& cameraPos, const glm::mat4& invViewProj, const glm::vec2& resolution)
	{
		wgfx::ComputePass computePass;
		computePass.prepare();

		glm::vec4 cameraPosData(cameraPos, 1.0f);
		compute->updateUniform(0, glm::value_ptr(cameraPosData));
		compute->updateUniform(2, glm::value_ptr(invViewProj));

		glm::vec4 resData(resolution.x, resolution.y, 0.0f, 0.0f);
		compute->updateUniform(3, glm::value_ptr(resData));
		compute->uniforms.clear();

		uint32_t workgroupsX = (uint32_t(resolution.x) + 7u) / 8u;
		uint32_t workgroupsY = (uint32_t(resolution.y) + 7u) / 8u;
		computePass.drawXY(compute, workgroupsX, workgroupsY);
		computePass.end();
	}

private:
	wgfx::Compute* compute;

	SphereCompute()
	{
		compute = wgfx::loadCompute(
			wgfx::loadFromFile((std::string(RESOURCE_DIR) + "/" + "sphere.wgsl").c_str()));

		outputTexture = wgfx::loadTextureSrc(raytraceWidth, raytraceHeight);
		compute->addUniform(0);
		compute->addStorageTexture(1, outputTexture);
		compute->addUniform(2);
		compute->addUniform(3);
		compute->init();
	}

	~SphereCompute() = default;

	SphereCompute(SphereCompute const&) = delete;
	void operator=(SphereCompute const&) = delete;
};
