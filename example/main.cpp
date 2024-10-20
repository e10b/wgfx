#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"

#include "clock.h"

int main()
{
	Context& context = Context::Instance();

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();

		context.update();
	}

}