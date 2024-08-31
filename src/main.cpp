#include "wgfx.h"

const char* shaderSource = R"(
/**
 * A structure with fields labeled with vertex attribute locations can be used
 * as input to the entry point of a shader.
 */
struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

/**
 * A structure with fields labeled with builtins and locations can also be used
 * as *output* of the vertex shader, which is also the input of the fragment
 * shader.
 */
struct VertexOutput {
	@builtin(position) position: vec4f,
	// The location here does not refer to a vertex attribute, it just means
	// that this field must be handled by the rasterizer.
	// (It can also refer to another field of another struct that would be used
	// as input to the fragment shader.)
	@location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	//                         ^^^^^^^^^^^^ We return a custom struct
	var out: VertexOutput; // create the output struct
	let ratio = 640.0 / 480.0; // The width and height of the target surface
	out.position = vec4f(in.position.x, in.position.y * ratio, 0.0, 1.0);
	out.color = in.color; // forward the color attribute to the fragment shader
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	//     ^^^^^^^^^^^^^^^^ Use for instance the same struct as what the vertex outputs
	return vec4f(in.color, 1.0); // use the interpolated color coming from the vertex shader
}
)";

std::vector<float> pointData = {
	// x,   y,     r,   g,   b
	-0.5, -0.5,   1.0, 0.0, 0.0, // Point #0
	+0.5, -0.5,   0.0, 1.0, 0.0, // Point #1
	+0.5, +0.5,   0.0, 0.0, 1.0, // Point #2
	-0.5, +0.5,   1.0, 1.0, 0.0  // Point #3
};

// Define index data
// This is a list of indices referencing positions in the pointData
std::vector<uint16_t> indexData = {
	0, 1, 2, // Triangle #0 connects points #0, #1 and #2
	0, 2, 3  // Triangle #1 connects points #0, #2 and #3
};
int main(int _argc, char** _argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) { return 1; }

	//SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE);
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);
	SDL_SetWindowAspectRatio(window, 16.f / 9.f, 16.f / 9.f);

	wgfx::init(wgfx::getSurface(window), 1280, 720);

	//wgfx::Program program = wgfx::loadProgram(shaderSource);
	wgfx::Program program = wgfx::loadProgram(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));

	wgfx::VertexBuffer vbo(pointData);
	vbo.setAttribute(0, wgfx::vec2f, 0);
	vbo.setAttribute(1, wgfx::vec3f, 2); // take in a type
	wgfx::IndexBuffer ibo(indexData);


	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	wgfx::Uniform uniform(1);
	program.linkUniform(uniform);
	//program.setUniform(uniform);

	bool shouldClose = false;
	while (!shouldClose)
	{
		SDL_Event event;
		float t = SDL_GetTicks() / 1000.0f;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
			{
					int newWidth = event.window.data1;
					int newHeight = event.window.data2;
					float aspectRatio = (float)newWidth / (float)newHeight;

					//wgfx::destroySurface();
					wgfx::initSurface();
					program.updateUniform(uniform, t);

					// Update viewport and aspect ratio uniform
					//wgfx::setViewport(0, 0, newWidth, newHeight);
					//wgfx::init(wgfx::getSurface(window), newWidth, newHeight);
					//program.updateUniform(aspectRatioUniform, aspectRatio);
				}
				break;

			case SDL_EVENT_QUIT:
				shouldClose = true;
				break;

			default:
				break;
			}
		}

		program.updateUniform(uniform, t);

		wgfx::submit(program);

	}

}