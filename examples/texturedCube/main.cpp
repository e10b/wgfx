#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <cmath>
#include <filesystem>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "helper/clock.h"
#include "helper/context.h"

namespace
{
std::vector<float> cubeVertexArray()
{
	return {
		1, -1, 1, 1, 1, 0, 1, 1, 0, 1, -1, -1, 1, 1, 0, 0, 1, 1, 1, 1, -1, -1, -1, 1, 0, 0, 0, 1, 1, 0,
		1, -1, -1, 1, 1, 0, 0, 1, 0, 0, 1, -1, 1, 1, 1, 0, 1, 1, 0, 1, -1, -1, -1, 1, 0, 0, 0, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, -1, 1, 1, 1, 0, 1, 1, 1, 1, 1, -1, -1, 1, 1, 0, 0, 1, 1, 0,
		1, 1, -1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, -1, -1, 1, 1, 0, 0, 1, 1, 0,
		-1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 0, 1, 1, 0,
		-1, 1, -1, 1, 0, 1, 0, 1, 0, 0, -1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, -1, 1, 1, 1, 0, 1, 1, 0,
		-1, -1, 1, 1, 0, 0, 1, 1, 0, 1, -1, 1, 1, 1, 0, 1, 1, 1, 1, 1, -1, 1, -1, 1, 0, 1, 0, 1, 1, 0,
		-1, -1, -1, 1, 0, 0, 0, 1, 0, 0, -1, -1, 1, 1, 0, 0, 1, 1, 0, 1, -1, 1, -1, 1, 0, 1, 0, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, -1, 1, 1, 1, 0, 1, 1, 1, 1, 1, -1, -1, 1, 1, 0, 0, 1, 1, 1, 0,
		-1, -1, 1, 1, 0, 0, 1, 1, 1, 0, 1, -1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
		1, -1, -1, 1, 1, 0, 0, 1, 0, 1, -1, -1, -1, 1, 0, 0, 0, 1, 1, 1, -1, 1, -1, 1, 0, 1, 0, 1, 1, 0,
		1, 1, -1, 1, 1, 1, 0, 1, 0, 0, 1, -1, -1, 1, 1, 0, 0, 1, 0, 1, -1, 1, -1, 1, 0, 1, 0, 1, 1, 0,
	};
}
}

int main()
{
	Context& context = Context::Instance();
	auto* color = new wgfx::ColorTexture();
	auto* depth = new wgfx::DepthTexture();
	auto* pass = new wgfx::RenderPass();
	pass->addTarget(color);
	pass->addTarget(depth);
	pass->setClear({0.5, 0.5, 0.5, 1.0});

	wgfx::Texture texture = wgfx::loadTexture(std::filesystem::path(RESOURCE_DIR) / "Di-3d.png");
	auto* pipeline = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "texturedCube.wgsl"));
	auto* vbo = wgfx::createVertexBuffer(cubeVertexArray());
	vbo->setAttribute(0, wgfx::vec4f, 0);
	vbo->setAttribute(1, wgfx::vec2f, 8);
	pipeline->addUniform(0);
	pipeline->addSampler(1, texture);
	pipeline->addTexture(2, texture);
	pipeline->cullMode = wgpu::CullMode::Back;
	pipeline->useDepth = true;
	pipeline->init(vbo);

	Clock clock;
	float now = 0.0f;
	while (!context.close) {
		now += clock.restart();
		context.update();
		wgfx::touch(color);
		wgfx::start();
		glm::mat4 projection = glm::perspective((2.0f * glm::pi<float>()) / 5.0f, 16.0f / 9.0f, 1.0f, 100.0f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
		view = glm::rotate(view, 1.0f, glm::normalize(glm::vec3(std::sin(now), std::cos(now), 0.0f)));
		glm::mat4 mvp = projection * view;
		pipeline->updateUniform(0, glm::value_ptr(mvp));
		pass->prepare();
		pipeline->setVertexBuffer(vbo);
		pass->draw(pipeline);
		pass->end();
		context.draw();
	}
}
