@group(0) @binding(0) var<uniform> view: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;
@group(0) @binding(2) var<uniform> proj: mat4x4f;

@group(0) @binding(3) var gradientTexture: texture_2d<f32>;
@group(0) @binding(4) var textureSampler: sampler;

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) color: vec3f,
	@location(2) uv: vec2f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
	@location(1) uv: vec2f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	
	out.position = proj * view * model * vec4f(in.position, 1.0);
	out.color = in.color;
	out.uv = in.uv * .999; //slight offset for graphical glitch -- because of the pixel data I think
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));
	
		//let color = textureLoad(gradientTexture, texelCoords, 0).rgb;
	let color = textureSample(gradientTexture, textureSampler, in.uv).rgb;

	let corrected_color = pow(color, vec3f(2.2));

	return vec4f(corrected_color, 1.0);
}