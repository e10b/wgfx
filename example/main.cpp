#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"


std::vector<float> random2 =
{
	0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,
1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0, 1.0, -1.0,
1.0, 1.0, 0.0, 0.0, -1.0, -1.0, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0, -1.0, 1.0, 1.0, -1.0,
1.0, 0.0, 1.0, 0.0, -1.0, -1.0, -1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
0.0, 0.0, 1.0, -1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0, 0.0,
-1.0, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, -1.0,
0.0, 0.0, 1.0, -1.0, 1.0, 0.0, -1.0, 1.0, 1.0, 1.0, 0.0, -1.0, 1.0, -1.0, -1.0, 0.0,
-1.0, 0.0, -1.0, -1.0, 1.0, 0.0, -1.0, -1.0, 1.0, 0.0, -1.0, 1.0, 1.0, -1.0, 0.0, 1.0,
1.0, 1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0,
-1.0, 1.0, -1.0, 1.0, 0.0, -1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0,
-1.0, 0.0, 0.0, -1.0, 1.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
-1.0, -1.0, 1.0, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0, -1.0

};



std::vector<uint16_t> random =
{
	8, 14, 10,
	22, 2, 10,
	4, 7, 17,
	18, 9, 0,
	13, 5, 13,
	6, 22, 12,
	18, 21, 5,
	11, 9, 14,
	15, 6, 20,
	17, 1, 1,
	2, 21, 23,
	3, 16, 19
};

#include "shader.h"
#include "cube.h"

void wall(wgfx::Pipeline* pipeline, wgfx::Uniform* uniform)
{
	std::cout << "is?\n";
	pipeline->setUniform(uniform, true);
}

int main()
{
	Context& context = Context::Instance();
	Input& input = Input::Instance();

	Player player;
	/*
	wgfx::Pipeline pipeline = wgfx::loadPipeline(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));
	wgfx::RenderPass renderPass;
	renderPass.setClear({ 0.0375, 0.0375, 0.0375, 1 });
	*/

	/*
	Shader shader("balls");
	wgfx::VertexBuffer vbo = wgfx::createVertexBuffer(pointData);
	vbo.setAttribute(0, wgfx::vec3f, 0); // position
	vbo.setAttribute(1, wgfx::vec3f, 3); // color
	vbo.setAttribute(2, wgfx::vec2f, 6); // uv
	shader.pipeline.setVertexBuffer(vbo);
	wgfx::IndexBuffer ibo = wgfx::createIndexBuffer(indexData);
	shader.pipeline.setIndexBuffer(ibo);

	wgfx::Uniform* view = wgfx::createUniform(0, sizeof(glm::mat4), 1.0f);	wall(&shader.pipeline, view);	//shader.pipeline.setUniform(*view, true);
	wgfx::Uniform* model = wgfx::createUniform(1, sizeof(glm::mat4), 1.0f); wall(&shader.pipeline, model);			//shader.pipeline.setUniform(*model, true);
	wgfx::Uniform* proj = wgfx::createUniform(2, sizeof(glm::mat4), 1.0f);	wall(&shader.pipeline, proj);	//shader.pipeline.setUniform(*proj, true);
	// maybe p[rehaps storing these objects as pointers is more logical, Struggle i find, struggle indeed. a most matron struggle, I feel like a dodojd diujh dij dij dij dij dij dd ijdd ijd 

	shader.pipeline.init(); // auto init?? well, 
	*/
	Cube cube;

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();
		//float time = SDL_GetTicks() / 1000.0f;

		context.update();

		input.update();
		player.update(dt);

		//draw
		const Camera& cam = player.getCamera();
		cube.draw(cam);
		/*
		const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
		if (keyboardState[SDL_SCANCODE_C] )
		{
			shader.pipeline.updateVertexBuffer(pointData);
			//shader.pipeline.set(indexData);
		}
		else
		{
			//shader.pipeline.updateVertexBuffer(random2);
		}

		shader.renderPass.touch();
		shader.pipeline.updateUniform(view, glm::value_ptr(cam.getViewMatrix()));
		shader.pipeline.updateUniform(proj, glm::value_ptr(cam.getProjectionMatrix()));
		// draw cubes
		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				shader.pipeline.updateUniform(model, glm::value_ptr(rotationMatrix));

				shader.use();//shader.renderPass.draw(shader.pipeline);
			}
		}
		shader.renderPass.end();
		*/
		context.draw();
	}

}