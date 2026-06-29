#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <array>
#include <filesystem>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "helper/clock.h"
#include "helper/constants.h"
#include "helper/context.h"

namespace
{
std::vector<float> makeCubeVertices()
{
	const float s = 0.75f;
	std::vector<float> vertices;
	vertices.reserve(24 * 5);

	auto pushVertex = [&](float x, float y, float z, float u, float v) {
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(u);
		vertices.push_back(v);
	};

	auto pushFace = [&](const std::array<glm::vec3, 4>& p) {
		pushVertex(p[0].x, p[0].y, p[0].z, 0.0f, 1.0f);
		pushVertex(p[1].x, p[1].y, p[1].z, 1.0f, 1.0f);
		pushVertex(p[2].x, p[2].y, p[2].z, 1.0f, 0.0f);
		pushVertex(p[3].x, p[3].y, p[3].z, 0.0f, 0.0f);
	};

	// Front
	pushFace({glm::vec3(-s, -s,  s), glm::vec3( s, -s,  s), glm::vec3( s,  s,  s), glm::vec3(-s,  s,  s)});
	// Back
	pushFace({glm::vec3( s, -s, -s), glm::vec3(-s, -s, -s), glm::vec3(-s,  s, -s), glm::vec3( s,  s, -s)});
	// Left
	pushFace({glm::vec3(-s, -s, -s), glm::vec3(-s, -s,  s), glm::vec3(-s,  s,  s), glm::vec3(-s,  s, -s)});
	// Right
	pushFace({glm::vec3( s, -s,  s), glm::vec3( s, -s, -s), glm::vec3( s,  s, -s), glm::vec3( s,  s,  s)});
	// Top
	pushFace({glm::vec3(-s,  s,  s), glm::vec3( s,  s,  s), glm::vec3( s,  s, -s), glm::vec3(-s,  s, -s)});
	// Bottom
	pushFace({glm::vec3(-s, -s, -s), glm::vec3( s, -s, -s), glm::vec3( s, -s,  s), glm::vec3(-s, -s,  s)});

	return vertices;
}

std::vector<uint16_t> makeCubeIndices()
{
	return {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
	};
}
} // namespace

int main()
{
	Context& context = Context::Instance();
	wgfx::ColorTexture* color = new wgfx::ColorTexture();
	wgfx::DepthTexture* depth = new wgfx::DepthTexture();
	wgfx::RenderPass* pass = new wgfx::RenderPass();
	pass->addTarget(color);
	pass->addTarget(depth);
	pass->setClear({0.02, 0.03, 0.05, 1.0});

	wgfx::Texture texture = wgfx::loadTexture(std::filesystem::path(RESOURCE_DIR) / "Di-3d.png");
	auto* pipeline = wgfx::loadPipeline(
		wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "texturedCube.wgsl"));
	auto* vbo = wgfx::createVertexBuffer(makeCubeVertices());
	vbo->setAttribute(0, wgfx::vec3f, 0);
	vbo->setAttribute(1, wgfx::vec2f, 3);
	auto* ibo = wgfx::createIndexBuffer(makeCubeIndices());
	pipeline->addUniform(0);
	pipeline->addSampler(1, texture);
	pipeline->addTexture(2, texture);
	pipeline->cullMode = wgpu::CullMode::Back;
	pipeline->useDepth = true;
	pipeline->targets = 1;
	pipeline->init(vbo);

	Clock clock;
	float rotation = 0.0f;

	while (!context.close)
	{
		float dt = clock.restart();
		rotation += dt;
		context.update();
		wgfx::touch(color);
		wgfx::start();

		int width = raytraceWidth;
		int height = raytraceHeight;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), float(width) / float(height), 0.1f, 20.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.2f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation * 0.9f, glm::vec3(0.4f, 1.0f, 0.2f));
		glm::mat4 mvp = projection * view * model;
		pipeline->updateUniform(0, glm::value_ptr(mvp));

		pass->prepare();
		pipeline->setVertexBuffer(vbo);
		pipeline->setIndexBuffer(ibo);
		pass->draw(pipeline);
		pass->end();

		context.draw();
	}

	return 0;
}
