#include "context.h"


int main(int _argc, char** _argv)
{
	Context context;
	//Shader shader("shaders");

	context.init();


	while (context.isRunning())
	{
		context.update();

		//shader.use();

		//context.draw();

	}

}