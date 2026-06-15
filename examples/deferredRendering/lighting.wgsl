@group(0) @binding(0) var gBufferNormal: texture_2d<f32>;
@group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;
@group(0) @binding(2) var gBufferDepth: texture_depth_2d;

struct LightData {
	position: vec4f,
	color: vec3f,
	radius: f32,
};

struct LightsBuffer {
	lights: array<LightData>,
};

@group(0) @binding(3) var<storage, read> lightsBuffer: LightsBuffer;

struct Config {
	numLights: u32,
};

struct Camera {
	viewProjectionMatrix: mat4x4f,
	invViewProjectionMatrix: mat4x4f,
};

@group(0) @binding(4) var<uniform> config: Config;
@group(0) @binding(5) var<uniform> camera: Camera;

fn world_from_screen_coord(coord: vec2f, depth_sample: f32) -> vec3f {
	let posClip = vec4f(coord.x * 2.0 - 1.0, (1.0 - coord.y) * 2.0 - 1.0, depth_sample, 1.0);
	let posWorldW = camera.invViewProjectionMatrix * posClip;
	return posWorldW.xyz / posWorldW.www;
}

@vertex
fn vs_main(@builtin(vertex_index) VertexIndex: u32) -> @builtin(position) vec4f {
	var pos = array<vec2f, 6>(
		vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
		vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
	);
	return vec4f(pos[VertexIndex], 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) coord: vec4f) -> @location(0) vec4f {
	var result = vec3f(0.0);
	let pixel = vec2i(floor(coord.xy));
	let depth = textureLoad(gBufferDepth, pixel, 0);
	if (depth >= 1.0) {
		discard;
	}

	let bufferSize = textureDimensions(gBufferDepth);
	let coordUV = coord.xy / vec2f(bufferSize);
	let position = world_from_screen_coord(coordUV, depth);
	let normal = textureLoad(gBufferNormal, pixel, 0).xyz;
	let albedo = textureLoad(gBufferAlbedo, pixel, 0).rgb;

	for (var i = 0u; i < config.numLights; i++) {
		let L = lightsBuffer.lights[i].position.xyz - position;
		let distance = length(L);
		if (distance > lightsBuffer.lights[i].radius) {
			continue;
		}
		let lambert = max(dot(normal, normalize(L)), 0.0);
		result += vec3f(
			lambert * pow(1.0 - distance / lightsBuffer.lights[i].radius, 2.0) * lightsBuffer.lights[i].color * albedo
		);
	}

	result += vec3f(0.2);
	return vec4f(result, 1.0);
}
