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
	Cube& cube = Cube();
	Cube& cube2 = Cube();
	cube.init(1);
	cube2.init(0); // color as i am sending 1 render target.

	wgfx::ColorTexture* color = new wgfx::ColorTexture();
	//wgfx::ColorTexture& color = wgfx::ColorTexture();
	wgfx::DepthTexture* depth = new wgfx::DepthTexture();
	wgfx::DepthTexture* depth1 = new wgfx::DepthTexture();
	//wgfx::DepthTexture& depth = wgfx::DepthTexture();

	Red red;
	Player player;

	wgfx::RenderPass pass; // only need the one member
	pass.addTarget(color);
	pass.addTarget(depth);



	wgfx::RenderPass pass2; // only need the one member
	pass2.addTarget(color);
	pass2.shouldClear = false;


wgfx::RenderPass p;
p.addTarget(depth1);


	Crosshair crosshair(depth1);
	//pass.prepareColor();
	//wgfx::initDepth();
	
	
	


	//Cube test;
	//wgfx::RenderPass pass;
pass.setClear({ 0.4, 0.7, 1, 1 });
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
		//pass.touch();
		wgfx::touch(color);
		

		p.prepare();
			//cube2.drawLit(cam.getViewMatrix(), p);
			glm::mat4 rotDown = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 view = cam.getViewMatrix();
			glm::mat4 rotatedView = rotDown * view;
			cube2.drawLit(rotatedView, p);

		p.end();

		pass.prepare();
			cube.drawLit(cam.getViewMatrix(), pass);
		pass.end();


		pass2.prepare();
			crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
			pass2.draw(crosshair.shader_.pipeline);
		pass2.end();


		// an interesting breakthrough, we do not need two renderpass member variables,
		// apparently we only need two if we are maintaining two distint passes at the same time
		// instead we end one and then begin a new one with the same member
		// calling analagous to pass = encoder.beginRenderPass(newPassDesc); << so the member is the same but the desc is diff.
		//pass.scene(false);
		//cube.drawLit(cam.getViewMatrix(), pass); // so here we are rendering depth to a texture. the texture is global, which is probably wrong.
		// the preferable way to do it is with clear definition and implementation.
		// so you would declare your pipeline and your renderpass. for the pipeline you might say that you are going to 
		// 
		//pass.end();





		//pass.scene(true);
		//glm::mat4 rotDown = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 view = cam.getViewMatrix();
		//glm::mat4 rotatedView = rotDown * view;

		//cube2.drawLit(rotatedView, pass); // so here we are rendering depth to a texture. the texture is global, which is probably wrong.
		// the preferable way to do it is with clear definition and implementation.
		// so you would declare your pipeline and your renderpass. for the pipeline you might say that you are going to 
		// 
		//pass.end();





		// render post-processing effects first
		//pass.post();
		//	crosshair.render(glm::vec2(1.f, cam.getAspect()) / 400.f);
		//pass.draw(crosshair.shader_.pipeline);
		//pass.end();

		/*pass.post();
			red.render(dt);
		pass.draw(red.shader.pipeline);
		pass.end();*/

		// Clear uniforms before scene rendering to reset buffer offsets

		// render scene depth then render scene.

		//cube.shader.pipeline->uniforms.clear(); right that was annoying

		/*pass.scene(false);
			cube2.drawLit(cam, pass); 
		pass.end();*/

		// now here i want to draw scene with color and depth, full draw.

		// notation?

		// targets.
		// okay so now i want to um, flip it.
		// so i want to render different views.

		// and the way i will do it is to render the depth from one static view
		// then render the scene proper. hmm. yes.

		// then i will layer them by.. uh idk.


		context.draw();
	}

}