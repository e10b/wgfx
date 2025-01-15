#pragma once

#include "shader.h"
#include "model.h"

#include "manager.h"

#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

class Selection
{
	Model mesh;
public:
	Shader shader;

	Selection()
	{

		shader = Shader("selection.wgsl");

		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
		vbo->setAttribute(0, wgfx::vec3f, 0); // position
		vbo->setAttribute(1, wgfx::vec3f, 3); // color
		vbo->setAttribute(2, wgfx::vec2f, 6); // uv
		vbo->setTopology(wgpu::PrimitiveTopology::LineList);

		shader.setVertexBuffer({ 0.0 });
		shader.setIndexBuffer({ 0 });
		//		shader.pipeline.setVertexBuffer(vbo);

		wgfx::IndexBuffer* ibo = wgfx::createIndexBuffer();
		//		shader.pipeline.setIndexBuffer(ibo);

		shader.setUniform(0); // camera
		shader.setUniform(1); // model

		shader.pipeline->init(vbo);
		//glm::mat4 proj = glm::perspective(glm::radians(90.0f)/*fov*/, float(1920) / 1080, 0.1f, 1000.0f); // reduce memory usage only send uniform for perspective once.
		//shader.setUniform(2, proj); // proj
		//shader.setTexture(3, texture); // tex wgfx::Uniform* sampler = wgfx::createTexture(3, texture);
		//shader.setSampler(4, texture); //sampler  wgfx::Uniform* tex = wgfx::createSampler(4, texture);

		//shader.pipeline.setTexture(sampler);
		//shader.pipeline.setSampler(tex);


		//shader.pipeline.init(); // auto init?? well, 
	}

	void draw(const Camera& camera, wgfx::RenderPass pass, glm::ivec3 pos)
	{
		//float time = SDL_GetTicks() / 1000.0f; // ought not
		//pos.y += 1;
		Manager& chunks = Manager::Instance();
		mesh.clear();
		for (int d = 0; d < Math::DIRECTION_COUNT; d++)
		{
			mesh.addQuad(quads[Math::Direction(d)], { 0,0 }, 0, pos, { 0,0,0 });
		}
		mesh.buffer(&mesh);
		//shader.renderPass.touch();
		glm::mat4 model = glm::mat4(1.0);
		//glm::mat4 model = glm::translate(glm::mat4(1.0), chunks.getChunk(pos)->getWorldPos());
		//glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(pos));
		//std::cout << "Model Matrix: " << glm::to_string(model) << std::endl;
		shader.updateUniform(0, camera.getMatrix());
		shader.updateUniform(1, model);
		//shader.updateUniform(2, camera.getProjectionMatrix());

		//for (uint32_t zz = 0; zz < 11; ++zz) {
		//	for (uint32_t yy = 0; yy < 11; ++yy) {
		//		for (uint32_t xx = 0; xx < 1; ++xx) {
		//			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), time + xx * 0.21f, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
		//			rotationMatrix = glm::rotate(rotationMatrix, time + yy * 0.37f, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
		//			rotationMatrix = glm::rotate(rotationMatrix, time + zz * 0.45f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis (new rotation for zz)

		//			rotationMatrix[3] = glm::vec4(-15.0f + float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, -15.0f + float(zz) * 3.0f, 1.0f); // Adjust position for zz
		//			//shader.pipeline.updateVertexBuffer(pointData);
		//			shader.updateUniform(1, rotationMatrix);
		//			shader.use();
		//		}
		//	}
		//}

		mesh.bind(shader.pipeline);
		shader.end();
		//glm::mat4 translation(1.0f);
		//translation = glm::translate(translation, glm::vec3(1.0f, 0.0f, 0.0f));
		//shader.updateUniform(1, translation);
		////shader.pipeline.updateVertexBuffer(pointData2);
		//shader.use();
		//shader.end();
		pass.draw(shader.pipeline);
//		shader.renderPass.end();
	}
};