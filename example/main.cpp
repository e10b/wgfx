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

int main()
{
	Context& context = Context::Instance();
	//Input& input = Input::Instance();

	Player player;

	//Cube cube;
	//Test test;

	Manager& manager = Manager::Instance();

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
		manager.drawChunks(cam);
		//cube.draw(cam);
		//test.render(cam);

		context.draw();
	}

}