const shadowDepthTextureSize: f32 = 1024.0;

struct Scene {
	lightViewProjMatrix: mat4x4f,
	cameraViewProjMatrix: mat4x4f,
	modelMatrix: mat4x4f,
	lightPos: vec4f,
};

@group(0) @binding(0) var<uniform> scene: Scene;
@group(0) @binding(1) var shadowMap: texture_depth_2d;
@group(0) @binding(2) var shadowSampler: sampler_comparison;

struct VertexOutput {
	@location(0) shadowPos: vec3f,
	@location(1) fragPos: vec3f,
	@location(2) fragNorm: vec3f,
	@builtin(position) Position: vec4f,
};

@vertex
fn vs_main(
	@location(0) position: vec3f,
	@location(1) normal: vec3f
) -> VertexOutput {
	let worldPosition = scene.modelMatrix * vec4f(position, 1.0);
	let posFromLight = scene.lightViewProjMatrix * worldPosition;

	var output: VertexOutput;
	output.shadowPos = vec3f(
		posFromLight.xy * vec2f(0.5, -0.5) + vec2f(0.5),
		posFromLight.z
	);
	output.Position = scene.cameraViewProjMatrix * worldPosition;
	output.fragPos = worldPosition.xyz;
	output.fragNorm = normal;
	return output;
}

const albedo = vec3f(0.9);
const ambientFactor = 0.2;

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
	var visibility = 0.0;
	let oneOverShadowDepthTextureSize = 1.0 / shadowDepthTextureSize;
	for (var y = -1; y <= 1; y++) {
		for (var x = -1; x <= 1; x++) {
			let offset = vec2f(vec2(x, y)) * oneOverShadowDepthTextureSize;
			visibility += textureSampleCompare(
				shadowMap,
				shadowSampler,
				input.shadowPos.xy + offset,
				input.shadowPos.z - 0.007
			);
		}
	}
	visibility /= 9.0;

	let lambertFactor = max(dot(normalize(scene.lightPos.xyz - input.fragPos), normalize(input.fragNorm)), 0.0);
	let lightingFactor = min(ambientFactor + visibility * lambertFactor, 1.0);
	return vec4f(lightingFactor * albedo, 1.0);
}
