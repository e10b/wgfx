#include "wgfx.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	-0.5, -0.5, -0.3,   1.0, 1.0, 1.0, // Point #0
	+0.5, -0.5, -0.3,   1.0, 1.0, 1.0, // Point #1
	+0.5, +0.5, -0.3,   1.0, 1.0, 1.0, // Point #2
	-0.5, +0.5, -0.3,   1.0, 1.0, 1.0,  // Point #3

	0, 0, 0.5,			0.5, 0.5, 0.5
};

// Define index data
// This is a list of indices referencing positions in the pointData
std::vector<uint16_t> indexData = {
	0, 1, 2, // Triangle #0 connects points #0, #1 and #2
	0, 2, 3,  // Triangle #1 connects points #0, #2 and #3

	0, 1, 4,
	1, 2, 4,
	2, 3, 4,
	3, 0, 4
};


float aa = 1;
int main(int _argc, char** _argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }

	//SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE);
	SDL_Window* window = SDL_CreateWindow("Learn WebGPU", 1280, 720, SDL_WINDOW_RESIZABLE);
	//SDL_SetWindowAspectRatio(window, 16.f / 9.f, 16.f / 9.f);

	wgfx::init(wgfx::getSurface(window), 1280, 720);

	//wgfx::Program program = wgfx::loadProgram(shaderSource);
	wgfx::Program program = wgfx::loadProgram(wgfx::loadFromFile(RESOURCE_DIR "/shader.wgsl"));

	wgfx::VertexBuffer vbo(pointData, 6);
	vbo.setAttribute(0, wgfx::vec3f, 0);
	vbo.setAttribute(1, wgfx::vec3f, 3); // take in a type
	wgfx::IndexBuffer ibo(indexData);


	float color[] = { 0.1, 0.2, 0.3, 0.4 };
	wgfx::Uniform uniform(0, sizeof(float), 1.0f);
	wgfx::Uniform uniform2(1, sizeof(color), color);
	
		glm::mat4 proj = glm::perspective(glm::radians(100.0f), float(1920) / 1080, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::rotate(view, glm::radians(80.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

		// Convert each matrix to float array
		float viewMatrix[16];
		float projMatrix[16];
		std::memcpy(viewMatrix, glm::value_ptr(view), 16 * sizeof(float));
		std::memcpy(projMatrix, glm::value_ptr(proj), 16 * sizeof(float));

	wgfx::Uniform viewUniform(2, sizeof(viewMatrix), viewMatrix);
	wgfx::Uniform projUniform(3, sizeof(projMatrix), projMatrix);

	program.setUniform(uniform);				// how can we me make it more natural<< i mean, uniform object so that we can update it. but wgfx::setUniform <<  
	program.setUniform(uniform2);
	program.setUniform(viewUniform);
	program.setUniform(projUniform);

	program.setVertexBuffer(vbo);
	program.setIndexBuffer(ibo);

	// uniforms are relative to the program << naturally.	// i am thinking a uniform a set and an update uniform makes the most sense to me in the natural way.

	//a single connective call should do.

	//program.linkUniforms();

	bool shouldClose = false;
	while (!shouldClose)
	{
		SDL_Event event;
		float t = SDL_GetTicks() / 1000.0f;
		float r = SDL_GetTicks() / 100.0f;
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

				case SDL_EVENT_WINDOW_EXPOSED:
				wgfx::initSurface();
				program.updateUniform(uniform, t);

			default:
				break;
			}
		}

		
		float d = 0.2;
		const Uint8* state = SDL_GetKeyboardState(NULL);

		if (state[SDL_SCANCODE_G]) {
			d = 0.4;
		}
		float cc[] = { 1.0, d, 1, 0.4 };
		view = glm::rotate(view, glm::radians(aa), glm::vec3(0.4f, 1.0f, 0.0f)); // Rotate 45 degrees around the Y-axis

		program.updateUniform(uniform, t);
		program.updateUniform(uniform2, cc);

		float v[16];
		std::memcpy(v, glm::value_ptr(view), 16 * sizeof(float));

		program.updateUniform(viewUniform, v);
		//program.updateUniform(uniform2, r);

		wgfx::submit(program);

	}

}