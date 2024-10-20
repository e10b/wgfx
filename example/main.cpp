#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include "context.h"
#include "clock.h"
#include "player.h"
#include "input.h"


std::vector<float> pointData =
{
	// x    y      z		nx    ny    nz     u     v
	-1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,  // Front face
	 1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,  // Back face
	 1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	 1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,  // Left face
	-1.0f, -1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,

	 1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,  // Right face
	 1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f,

	-1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,  // Top face
	 1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,  // Bottom face
	 1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f
};

std::vector<uint16_t> indexData =
{
	0, 1, 2,
	1, 3, 2,
	4, 5, 6,
	5, 7, 6,
	8, 9, 10,
	9, 11, 10,
	12, 13, 14,
	13, 15, 14,
	16, 17, 18,
	17, 19, 18,
	20, 21, 22,
	21, 23, 22
};

#include "shader.h"

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
	Shader shader("balls");

	wgfx::VertexBuffer vbo = wgfx::createVertexBuffer(pointData);
	vbo.setAttribute(0, wgfx::vec3f, 0); // position
	vbo.setAttribute(1, wgfx::vec3f, 3); // color
	vbo.setAttribute(2, wgfx::vec2f, 6); // uv
	wgfx::IndexBuffer ibo = wgfx::createIndexBuffer(indexData);

	wgfx::Uniform viewUniform = wgfx::createUniform(0, sizeof(glm::mat4), 1.0f);     shader.pipeline.setUniform(viewUniform, true);
	wgfx::Uniform modelUniform = wgfx::createUniform(1, sizeof(glm::mat4), 1.0f);	 shader.pipeline.setUniform(modelUniform, true);
	wgfx::Uniform projUniform = wgfx::createUniform(2, sizeof(glm::mat4), 1.0f);	 shader.pipeline.setUniform(projUniform, true);

	shader.pipeline.setVertexBuffer(vbo);
	shader.pipeline.setIndexBuffer(ibo);

	while (!context.close)
	{
		static Clock clock;
		float dt = clock.restart();
		float time = SDL_GetTicks() / 1000.0f;

		context.update();

		input.update();
		player.update(dt);

		//draw
		const Camera& cam = player.getCamera();

		shader.renderPass.touch();
		shader.pipeline.updateUniform(viewUniform, glm::value_ptr(cam.getViewMatrix()));
		shader.pipeline.updateUniform(projUniform, glm::value_ptr(cam.getProjectionMatrix()));
		// draw cubes
		for (uint32_t yy = 0; yy < 11; ++yy) {
			for (uint32_t xx = 0; xx < 11; ++xx) {
				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
				rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
				rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f, 1.0f);
				shader.pipeline.updateUniform(modelUniform, glm::value_ptr(rotationMatrix));

				shader.use();//shader.renderPass.draw(shader.pipeline);
			}
		}
		shader.renderPass.end();
		context.draw();
	}

}