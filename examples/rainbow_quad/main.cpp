#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <vector>
#include <string>

#include "helper/clock.h"
#include "helper/constants.h"
#include "helper/context.h"

namespace
{
std::vector<float> makeRainbowQuadVertices()
{
	return {
		-0.75f, -0.75f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.75f, -0.75f, 0.0f, 1.0f, 1.0f, 0.0f,
		 0.75f,  0.75f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.75f,  0.75f, 0.0f, 0.9f, 0.2f, 1.0f,
	};
}

std::vector<uint16_t> makeQuadIndices()
{
	return {0, 1, 2, 0, 2, 3};
}
} // namespace

int main()
{
	Context& context = Context::Instance();
	wgfx::ColorTexture* color = new wgfx::ColorTexture();
	wgfx::RenderPass* pass = new wgfx::RenderPass();
	pass->addTarget(color);
	pass->setClear({0.06, 0.07, 0.10, 1.0});

	auto* pipeline = wgfx::loadPipeline(wgfx::loadFromFile((std::string(RESOURCE_DIR) + "/rainbow_quad.wgsl").c_str()));
	auto* vbo = wgfx::createVertexBuffer(makeRainbowQuadVertices());
	vbo->setAttribute(0, wgfx::vec3f, 0);
	vbo->setAttribute(1, wgfx::vec3f, 3);
	auto* ibo = wgfx::createIndexBuffer(makeQuadIndices());
	pipeline->useDepth = false;
	pipeline->targets = 1;
	pipeline->init(vbo);

	while (!context.close)
	{
		context.update();
		wgfx::touch(color);
		wgfx::start();

		pass->prepare();
		pipeline->setVertexBuffer(vbo);
		pipeline->setIndexBuffer(ibo);
		pass->draw(pipeline);
		pass->end();

		context.draw();
	}

	return 0;
}