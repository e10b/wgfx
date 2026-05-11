@group(0) @binding(0) var texture0 : texture_2d<f32>;
@group(0) @binding(1) var sampler0 : sampler;

struct VertexInput {
	@location(0) position: vec3<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) uv: vec2<f32>,
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
	var output : VertexOutput;
	output.position = vec4<f32>(input.position, 1.0);
	output.uv = input.position.xy * 0.5 + vec2<f32>(0.5, 0.5);
	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	return textureSample(texture0, sampler0, input.uv);
}