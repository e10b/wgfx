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
#include "shader.h"
// quick red quad

class Red
{
public:
	Shader shader;
	Model model;
	Red()
		:shader("red.wgsl")
	{
		float quadWidth = 0.3f;
		float quadHeight = 0.3f;

		// Offset from center to bottom-right
		float x = 1.0f - quadWidth - 0.5;
		float y = -1.0f + quadHeight;

		model.addData({
			//   x,             y,            z,      u,    v
			-quadWidth + x, -quadHeight + y, 0.0f,   0.0f, 1.0f, // bottom-left
			 quadWidth + x, -quadHeight + y, 0.0f,   1.0f, 1.0f, // bottom-right
			 quadWidth + x,  quadHeight + y, 0.0f,   1.0f, 0.0f, // top-right
			-quadWidth + x,  quadHeight + y, 0.0f,   0.0f, 0.0f  // top-left
			}, {
				0, 1, 2,
				0, 2, 3
			});

			wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
			vbo->setAttribute(0, wgfx::vec3f, 0); // pos
			vbo->setAttribute(1, wgfx::vec2f, 3); // uv
			shader.setVertexBuffer({ 0.0 });
			shader.setIndexBuffer({ 0 });
			shader.setUniform(0); // time
			shader.pipeline->useDepth = false;
			
			shader.pipeline->init(vbo);
	}
	void render(float dt)
	{
		shader.end();
		
		shader.updateUniform(0, glm::vec2(dt,0));
		model.bind(shader.pipeline);
	}
};


int main()
{
	Context& context = Context::Instance();
	Cube& cube = Cube::Instance();
	Red red;
	Player player;

	wgfx::RenderPass pass; // only need the one member
	//pass.prepareColor();
	wgfx::initDepth();
	
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

		pass.post();
		red.render(dt);
		pass.draw(red.shader.pipeline);
		pass.end();




		context.draw();
	}

}