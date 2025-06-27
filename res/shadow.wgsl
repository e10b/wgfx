@group(0) @binding(0) var<uniform> view: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;
@group(0) @binding(2) var<uniform> proj: mat4x4f;


struct VertexInput {
	@location(0) position: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
};

@vertex 
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4f(input.position, 1.0);
    return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    return vec4f(.2, .2, .2, 1.0);
}