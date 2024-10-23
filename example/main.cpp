#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"

#include "cube.h"

int main()
{
	Context& context = Context::Instance();
	Input& input = Input::Instance();

	Player player;
	
	Cube cube;

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();

		context.update();

		input.update();
		player.update(dt);

		//draw
		const Camera& cam = player.getCamera();
		cube.draw(cam);
		
		context.draw();
	}

}