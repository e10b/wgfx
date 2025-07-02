#pragma once

#include <glm/glm.hpp>

#include "shader.h"
#include "model.h"

#include "wgfx.h"

class Crosshair
{
public:

	wgfx::Texture depth;


	void setDepthTexture()
	{
		depth = wgfx::loadTexture(wgfx::depthTextureView);
		shader_.setDepthTexture(1, depth);
		shader_.setSampler(2, depth);
	}
		
	Crosshair(wgfx::DepthTexture* depthTex)
		:shader_("crosshair.wgsl")
	{


		float quadWidth = 0.3f;
		float quadHeight = 0.3f;

		// Offset from center to bottom-right
		float x = 1.0f - quadWidth;
		float y = -1.0f + quadHeight;

		cross_.addData({
			//   x,             y,            z,      u,    v
			-quadWidth + x, -quadHeight + y, 0.0f,   0.0f, 1.0f, // bottom-left
			 quadWidth + x, -quadHeight + y, 0.0f,   1.0f, 1.0f, // bottom-right
			 quadWidth + x,  quadHeight + y, 0.0f,   1.0f, 0.0f, // top-right
			-quadWidth + x,  quadHeight + y, 0.0f,   0.0f, 0.0f  // top-left
			}, {
				0, 1, 2,
				0, 2, 3
			});

		depth = wgfx::loadTexture(depthTex->depthView);
		wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
		vbo->setAttribute(0, wgfx::vec3f, 0); // pos
		vbo->setAttribute(1, wgfx::vec2f, 3); // uv
		shader_.setVertexBuffer({ 0.0 });
		shader_.setIndexBuffer({ 0 });

		shader_.setUniform(0); // size
		//shader_.setUniform(1); // size

		shader_.pipeline->targets = 1;
		shader_.pipeline->useDepth = false;

		//shader_.pipeline->multiTarget = false;

		//depth = wgfx::loadTexture(RESOURCE_DIR "/crate.png");

		//depth = wgfx::loadTexture(wgfx::offscreenView);
		shader_.setDepthTexture(1, depth);
		shader_.setSampler(2, depth);
		
		
		//shader_.setDepthTexture(1, depth);
		//shader_.setTexture();

		shader_.pipeline->init(vbo);
	}

	void render(glm::vec2 size)
	{
		//shader_.touch();
		//shader_.updateUniform(0, size); // size
		shader_.end();

		shader_.updateUniform(0, size);

		/*if (wgfx::resetDepth)
		{
			std::cout << "UPDATATING DEPTH!\n";
			depth = wgfx::loadTexture(wgfx::depthTextureView);
			shader_.updateTexture(1, depth);
		}*/ //sod off

		/*cross_.addData({ -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f }
		, { 0,1,2,3,2,1 });*/
		

		cross_.bind(shader_.pipeline);
		//shader_.use();
	}

	Shader shader_;
private:
	Model cross_;
};