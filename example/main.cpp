#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"

#include "cube.h"

#include "test.h"

#include "manager.h"

//#include <omp.h>

#include "crosshair.h"


int main()
{
	Context& context = Context::Instance();
	//Input& input = Input::Instance();

	Player player;

	//Cube cube;
	//Test test;

	Manager& manager = Manager::Instance();

	Crosshair crosshair;

	wgfx::RenderPass pass;
	pass.setClear({ 0.4, 0.7, 1, 1 });
	int boo = 0;

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();

		if (boo % 400 == 0)
		{
			context.fps(dt);
			boo++;
		}
		boo++;

		//update
#pragma omp parallel sections
		{
#pragma omp section
			{
				context.update();
			}
#pragma omp section
			{
				player.update(dt);
			}
#pragma omp section
			{
				manager.updateChunks(player.getCamera().getPosition(), dt);
			}
		}
		pass.touch();

		const Camera& cam = player.getCamera();
		manager.drawChunks(cam, pass);
		//cube.draw(cam);
		//test.render(cam);
		//pass.draw(manager.shader_.pipeline);
		manager.shader_.pipeline->index++;
		wgfx::clear();

		//context.draw();

		crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
		pass.draw(crosshair.shader_.pipeline);
		wgfx::clear();

		pass.end();

		context.draw();
	}

}