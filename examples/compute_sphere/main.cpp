#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "helper/clock.h"
#include "helper/constants.h"
#include "helper/context.h"
#include "helper/quad.h"
#include "helper/sphere_compute.h"

int main()
{
	Context& context = Context::Instance();
	Quad& quad = Quad::Instance();
	SphereCompute& sphereCompute = SphereCompute::Instance();

	wgfx::ColorTexture* color = new wgfx::ColorTexture();
	wgfx::RenderPass* pass = new wgfx::RenderPass();
	pass->addTarget(color);
	pass->setClear({0.04, 0.05, 0.08, 1.0});

	while (!context.close)
	{
		context.update();
		wgfx::touch(color);
		wgfx::start();

		sphereCompute.render(glm::vec3(0.0f), glm::mat4(1.0f), glm::vec2(raytraceWidth, raytraceHeight));
		wgfx::copyTextureToTexture(&sphereCompute.outputTexture, &quad.texture);

		pass->prepare();
		quad.render();
		pass->draw(quad.pipeline);
		pass->end();

		context.draw();
	}

	return 0;
}