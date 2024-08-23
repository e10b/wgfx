#include "context.h"

#include "shader.h"

const char* sr = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

int main(int _argc, char** _argv)
{
	Context& context = Context::getInstance();
	context.init();

	Shader shader(sr);

	while (context.isRunning())
	{
		//context.update();

		context.encode();

		shader.use();
		
		context.submit();


		//context.draw();

	}

}