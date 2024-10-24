#pragma once

#include "model.h"
#include "shader.h"

#include "camera.h"
#include "texture.h"

#include "chunk.h"
#include "terrain.h"

class Test
{
public:
	Test()
		: shader_("shader"),
		chunk_({0,0})
	{
		texture = wgfx::loadTexture(RESOURCE_DIR "/crate2.jpg");
		chunk_.generate(terrain_);
		chunk_.buildMesh();
		chunk_.draw(&shader_.pipeline);
	
		shader_.setUniform(0); // camera
		shader_.setTexture(1, texture); // tex
		shader_.setSampler(2, texture); // sampler
	
		shader_.pipeline.init(); // auto init?? well, 

	}

	void render(const Camera& cam)
	{
		shader_.renderPass.touch();

		shader_.updateUniform(0, cam.getMatrix());

		shader_.use();

		shader_.renderPass.end();
		
	}

private:
	Chunk chunk_;
	Terrain terrain_;
	Shader shader_;
	wgfx::Texture texture;
};
