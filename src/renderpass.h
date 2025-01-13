#pragma once

#include "pipeline.h"

namespace wgfx
{
	struct RenderPass
	{
		RenderPassEncoder renderPass = nullptr;
		WGPUColor clearValue;

		void setIndex() // make this index a better name or abstract, this is not obvious what it's for (save the ambiguous comment)
		{
			for (auto p : pipelines) // neccessary to loop through setters for vbos/ibos
			{
				p->index = 0;
			}
		}

		void end();
		void setClear(WGPUColor color);
		void draw(Pipeline* pipeline);


		void touch();

	};
	TextureView getNextSurfaceTextureView();
}