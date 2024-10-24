#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"

#include "cube.h"

#include "test.h"

int main()
{
	Context& context = Context::Instance();
	Input& input = Input::Instance();

	Player player;
	
	//Cube cube;
	Test test;

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();

		context.update();

		input.update();
		player.update(dt);

		const Camera& cam = player.getCamera();
		//cube.draw(cam);
		test.render(cam);

		context.draw();
	}

}