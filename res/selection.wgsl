@group(0) @binding(0) var<uniform> model: mat4x4f;
@group(0) @binding(1) var<uniform> camera: mat4x4f;


struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;

	let offset = in.normal * 0.005;
	out.position = camera * ( model * vec4f(in.position + offset, 1.0) );

	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let color = vec3(0.0f, 0.0f, 0.0);

	let corrected_color = pow(color, vec3f(2.2));

	return vec4f(corrected_color, 1.0);
}