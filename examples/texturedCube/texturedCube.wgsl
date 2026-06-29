struct VertexInput {
	@location(0) position: vec3<f32>,
	@location(1) uv: vec2<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) uv: vec2<f32>,
};

@group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;
@group(0) @binding(1) var cubeSampler: sampler;
@group(0) @binding(2) var cubeTexture: texture_2d<f32>;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
	var output: VertexOutput;
	output.position = mvp * vec4<f32>(input.position, 1.0);
	output.uv = input.uv;
	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	return textureSample(cubeTexture, cubeSampler, input.uv);
}
