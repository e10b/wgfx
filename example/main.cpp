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

	wgfx::RenderPass pass; // only need the one member
	pass.prepareColor();

	Crosshair crosshair;
	
	
	pass.setClear({ 0.4, 0.7, 1, 1 });


	//Cube test;
	//wgfx::RenderPass pass;
	//pass.setClear({ 0.4, 0.7, 1, 1 });
	//int boo = 0;
	std::cout << "waaa\n";

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
		pass.touch();
		pass.scene();

		//manager.drawChunksLit(cam, pass);

		//selection.draw(cam, pass, player.selectionPos);
		//test.drawLit(cam);
		cube.drawLit(cam, pass);

		//crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
		pass.end();

		// an interesting breakthrough, we do not need two renderpass member variables,
		// apparently we only need two if we are maintaining two distint passes at the same time
		// instead we end one and then begin a new one with the same member
		// calling analagous to pass = encoder.beginRenderPass(newPassDesc); << so the member is the same but the desc is diff.


		pass.post();
		crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
		pass.draw(crosshair.shader_.pipeline);

		pass.end();




		context.draw();
	}

}