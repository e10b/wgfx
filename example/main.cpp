#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"


#include "test.h"

#include "manager.h"
#include "crosshair.h"
//#include <omp.h>
#include "selection.h"

int main()
{
	Context& context = Context::Instance();
	//Input& input = Input::Instance();

	Player player;
	Crosshair crosshair;
	Selection selection;

	//Cube cube;
	//Test test;

	Manager& manager = Manager::Instance();

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

		const Camera& cam = player.getCamera();
		pass.touch();

		manager.drawChunksLit(cam, pass);

		selection.draw(cam, pass, player.selectionPos);

		//cube.draw(cam);
		//test.render(cam);

		// problem area, with overrunning offset for bind group 0 bind 0. for dynamic uniform. ....
					crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f); 
					pass.draw(crosshair.shader_.pipeline);
		// problem a

		pass.end();
		context.draw();
	}

}