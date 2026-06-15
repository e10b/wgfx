struct Uniforms {
	modelMatrix: mat4x4f,
	normalModelMatrix: mat4x4f,
	viewProjectionMatrix: mat4x4f,
	invViewProjectionMatrix: mat4x4f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

struct VertexOutput {
	@builtin(position) Position: vec4f,
	@location(0) fragNormal: vec3f,
	@location(1) fragUV: vec2f,
};

@vertex
fn vs_main(
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) uv: vec2f
) -> VertexOutput {
	let worldPosition = (uniforms.modelMatrix * vec4f(position, 1.0)).xyz;
	var output: VertexOutput;
	output.Position = uniforms.viewProjectionMatrix * vec4f(worldPosition, 1.0);
	output.fragNormal = normalize((uniforms.normalModelMatrix * vec4f(normal, 1.0)).xyz);
	output.fragUV = uv;
	return output;
}

struct GBufferOutput {
	@location(0) normal: vec4f,
	@location(1) albedo: vec4f,
};

@fragment
fn fs_main(
	@location(0) fragNormal: vec3f,
	@location(1) fragUV: vec2f
) -> GBufferOutput {
	let uv = floor(30.0 * fragUV);
	let c = 0.2 + 0.5 * ((uv.x + uv.y) - 2.0 * floor((uv.x + uv.y) / 2.0));

	var output: GBufferOutput;
	output.normal = vec4f(normalize(fragNormal), 1.0);
	output.albedo = vec4f(c, c, c, 1.0);
	return output;
}
