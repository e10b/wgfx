#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <cmath>
#include <filesystem>
#include <array>
#include <vector>

#include "helper/clock.h"
#include "helper/context.h"

namespace
{
constexpr uint32_t kBoidCount = 256;

struct Params {
	float dt;
	float count;
	float aspect;
	float time;
};

struct Boid {
	glm::vec2 pos;
	glm::vec2 vel;
};

std::vector<Boid> makeBoids()
{
	std::vector<Boid> boids(kBoidCount);
	for (uint32_t i = 0; i < kBoidCount; ++i) {
		float a = float(i) * 2.3999632f;
		float r = 0.15f + 0.75f * float(i % 37) / 37.0f;
		boids[i].pos = glm::vec2(std::cos(a) * r * 1.35f, std::sin(a) * r);
		boids[i].vel = glm::normalize(glm::vec2(-std::sin(a), std::cos(a))) * (0.22f + 0.15f * float(i % 11) / 11.0f);
	}
	return boids;
}
}

int main()
{
	Context& context = Context::Instance();
	auto* color = new wgfx::ColorTexture();
	auto* pass = new wgfx::RenderPass();
	pass->addTarget(color);
	pass->setClear({0.008, 0.010, 0.016, 1.0});

	std::vector<Boid> boids = makeBoids();
	const std::string shader = wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "boids.wgsl");

	auto* computeA = wgfx::loadCompute(shader);
	computeA->entryPoint = "cs_main";
	computeA->addUniform(0);
	auto* aSrc = computeA->addStorage(1, sizeof(Boid) * boids.size(), boids.data(), true);
	auto* aDst = computeA->addStorage(2, sizeof(Boid) * boids.size(), boids.data());
	computeA->init();

	auto* computeB = wgfx::loadCompute(shader);
	computeB->entryPoint = "cs_main";
	computeB->addUniform(0);
	auto* bSrc = computeB->addStorage(1, sizeof(Boid) * boids.size(), nullptr, true);
	auto* bDst = computeB->addStorage(2, sizeof(Boid) * boids.size(), nullptr);
	bSrc->buffer = aDst->buffer;
	bSrc->entry.buffer = aDst->buffer;
	bDst->buffer = aSrc->buffer;
	bDst->entry.buffer = aSrc->buffer;
	computeB->init();

	auto* renderA = wgfx::loadPipeline(shader);
	renderA->targets = 1;
	renderA->useDepth = false;
	renderA->vertices = 3;
	renderA->instances = kBoidCount;
	renderA->addUniform(0, sizeof(Params));
	renderA->uniforms.visibility = wgpu::ShaderStage::Vertex;
	renderA->uniforms.setStorage(wgfx::createStorage(1, sizeof(Boid) * boids.size(), nullptr, true));
	renderA->uniforms.uniforms.back()->buffer = aSrc->buffer;
	renderA->uniforms.entries.back().buffer = aSrc->buffer;
	renderA->uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
	renderA->init(nullptr);

	auto* renderB = wgfx::loadPipeline(shader);
	renderB->targets = 1;
	renderB->useDepth = false;
	renderB->vertices = 3;
	renderB->instances = kBoidCount;
	renderB->addUniform(0, sizeof(Params));
	renderB->uniforms.visibility = wgpu::ShaderStage::Vertex;
	renderB->uniforms.setStorage(wgfx::createStorage(1, sizeof(Boid) * boids.size(), nullptr, true));
	renderB->uniforms.uniforms.back()->buffer = aDst->buffer;
	renderB->uniforms.entries.back().buffer = aDst->buffer;
	renderB->uniforms.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
	renderB->init(nullptr);

	wgfx::ComputePass computePass;
	Clock clock;
	float time = 0.0f;
	bool flip = false;

	while (!context.close) {
		float dt = std::min(clock.restart(), 0.033f);
		time += dt;
		context.update();
		wgfx::touch(color);
		wgfx::start();

		Params params{dt, float(kBoidCount), 16.0f / 9.0f, time};
		std::array<float, 16> computeParams{};
		computeParams[0] = params.dt;
		computeParams[1] = params.count;
		computeParams[2] = params.aspect;
		computeParams[3] = params.time;
		auto* compute = flip ? computeB : computeA;
		compute->updateUniform(0, computeParams.data());
		computePass.prepare();
		computePass.draw(compute, (kBoidCount + 63) / 64);
		computePass.end();

		auto* render = flip ? renderB : renderA;
		render->updateUniform(0, reinterpret_cast<const float*>(&params));
		pass->prepare();
		pass->draw(render);
		pass->end();
		context.draw();
		flip = !flip;
	}
}
