#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"

#include "cube.h"


#include "manager.h"
#include "crosshair.h"
//#include <omp.h>
#include "selection.h"


int main()
{
	Context& context = Context::Instance();
	Cube& cube = Cube::Instance();
	Player player;
	//Cube test;
	//wgfx::RenderPass pass;
	//pass.setClear({ 0.4, 0.7, 1, 1 });
	//int boo = 0;

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();

		//if (boo % 400 == 0)
		//{
		//	context.fps(dt);
		//	boo++;
		//}
		//boo++;
		
		//update
				context.update();
				player.update(dt);
				//manager.updateChunks(player.getCamera().getPosition(), dt);

		const Camera& cam = player.getCamera();
		//pass.touch();

		//manager.drawChunksLit(cam, pass);

		//selection.draw(cam, pass, player.selectionPos);
		//test.drawLit(cam);
		cube.drawLit(cam);
		//cube.draw(cam);
		//test.draw(cam, pass);

		// problem area, with overrunning offset for bind group 0 bind 0. for dynamic uniform. ....
		//crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
		//pass.draw(crosshair.shader_.pipeline);
		// problem a

		//pass.end();



		context.draw();
	}

}