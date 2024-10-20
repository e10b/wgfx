@group(0) @binding(0) var<uniform> view: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;
@group(0) @binding(2) var<uniform> proj: mat4x4f;

//@group(0) @binding(3) var gradientTexture: texture_2d<f32>;
//@group(0) @binding(4) var textureSampler: sampler;

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) uv: vec2f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,
	@location(1) uv: vec2f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	
	out.position = proj * view * model * vec4f(in.position, 1.0);
	//out.color = in.color;
	    out.normal = (model * vec4f(in.normal, 0.0)).xyz;

	out.uv = in.uv;			//slight offset for graphical glitch -- * .99 might be necessary -- maybe not
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	
	let normal = normalize(in.normal);

	let color = vec3(1.0f, 1.0f, 1.0f);

	let corrected_color = pow(color, vec3f(2.2));

	return vec4f(corrected_color, 1.0);
}