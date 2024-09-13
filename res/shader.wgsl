@group(0) @binding(0) var<uniform> view: mat4x4f;
@group(0) @binding(1) var<uniform> proj: mat4x4f;
@group(0) @binding(2) var<uniform> model: mat4x4f;

struct VertexInput {
	@location(0) position: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	
	out.position = proj * view * model * vec4f(in.position, 1.0);

	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {


	return vec4f(1.0, 1.0, 1.0, 1.0);
}