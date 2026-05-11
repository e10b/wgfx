#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <array>
#include <filesystem>
#include <vector>
#include <string>

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
	// One color per cube corner (8 corners, rainbow palette)
	const std::array<glm::vec3, 8> cornerColors = {
		glm::vec3(1.0f, 0.0f, 0.0f),		// corner 0: (-,-,-) red
		glm::vec3(1.0f, 0.5f, 0.0f),		// corner 1: (+,-,-) orange
		glm::vec3(1.0f, 1.0f, 0.0f),		// corner 2: (+,+,-) yellow
		glm::vec3(0.0f, 1.0f, 0.0f),		// corner 3: (-,+,-) green
		glm::vec3(0.0f, 1.0f, 1.0f),		// corner 4: (-,-,+) cyan
		glm::vec3(0.0f, 0.0f, 1.0f),		// corner 5: (+,-,+) blue
		glm::vec3(1.0f, 0.0f, 1.0f),		// corner 6: (+,+,+) magenta
		glm::vec3(1.0f, 1.0f, 1.0f),		// corner 7: (-,+,+) white
	};

	std::vector<float> vertices;
	vertices.reserve(24 * 6);

	auto getCornerColor = [&](float x, float y, float z) -> glm::vec3 {
		// Map coordinate to corner index (0-7)
		int idx = 0;
		if (x > 0) idx |= 1;
		if (y > 0) idx |= 2;
		if (z > 0) idx |= 4;
		return cornerColors[idx];
	};

	auto pushVertex = [&](float x, float y, float z) {
		const glm::vec3 color = getCornerColor(x, y, z);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(color.r);
		vertices.push_back(color.g);
		vertices.push_back(color.b);
	};

	// Front
	pushVertex(-s, -s,  s);
	pushVertex( s, -s,  s);
	pushVertex( s,  s,  s);
	pushVertex(-s,  s,  s);
	// Back
	pushVertex( s, -s, -s);
	pushVertex(-s, -s, -s);
	pushVertex(-s,  s, -s);
	pushVertex( s,  s, -s);
	// Left
	pushVertex(-s, -s, -s);
	pushVertex(-s, -s,  s);
	pushVertex(-s,  s,  s);
	pushVertex(-s,  s, -s);
	// Right
	pushVertex( s, -s,  s);
	pushVertex( s, -s, -s);
	pushVertex( s,  s, -s);
	pushVertex( s,  s,  s);
	// Top
	pushVertex(-s,  s,  s);
	pushVertex( s,  s,  s);
	pushVertex( s,  s, -s);
	pushVertex(-s,  s, -s);
	// Bottom
	pushVertex(-s, -s, -s);
	pushVertex( s, -s, -s);
	pushVertex( s, -s,  s);
	pushVertex(-s, -s,  s);

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

	auto* pipeline = wgfx::loadPipeline(
		wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "spinning_cube.wgsl"));
	auto* vbo = wgfx::createVertexBuffer(makeCubeVertices());
	vbo->setAttribute(0, wgfx::vec3f, 0);
	vbo->setAttribute(1, wgfx::vec3f, 3);
	auto* ibo = wgfx::createIndexBuffer(makeCubeIndices());
	pipeline->addUniform(0);
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