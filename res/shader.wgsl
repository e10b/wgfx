@group(0) @binding(0) var<uniform> uTime: f32;

@group(0) @binding(1) var<uniform> uColor: vec3f;

@group(0) @binding(2) var<uniform> view: mat4x4f;
@group(0) @binding(3) var<uniform> proj: mat4x4f;

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) color: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let ratio = 1920.0 / 1080.0;
	var offset = vec2f(-0.6875 * 0, -0.463 * 0);
	offset += 0.3 * vec2f(cos(uTime), sin(uTime));
		offset = vec2f(0, 0);

		//let homogeneous_position = vec4f(in.position, 1.0);
		//let position = (proj * view * homogeneous_position).xyz;

	out.position = proj * view * vec4f(in.position, 1.0);

	
	
	out.color = in.color;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let color = in.color * uColor.rgb;
	let corrected_color = pow(color, vec3f(2.2));

	return vec4f(corrected_color.rgb, 1.0);
}