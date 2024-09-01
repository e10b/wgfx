// Variable in the *uniform* address space
// The memory location of the uniform is given by a pair of a *bind group* and
// a *binding*.
@group(0) @binding(0) var<uniform> uTime: f32;

@group(0) @binding(1) var<uniform> uTime2: f32;

struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let ratio = 640.0 / 480.0;
	// We move the scene depending on the time
	var offset = vec2f(-0.6875 * 0, -0.463 * 0);
	offset += 0.3 * vec2f(cos(uTime), sin(uTime) * tan(uTime2));
	out.position = vec4f(in.position.x + offset.x, (in.position.y + offset.y) * ratio, 0.0, 1.0);
	out.color = in.color;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	// We apply a gamma-correction to the color
	let corrected_color = pow(in.color, vec3f(2.2));
	return vec4f(corrected_color, 1.0);
}