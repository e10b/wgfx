@group(0) @binding(0) var gBufferNormal: texture_2d<f32>;
@group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;
@group(0) @binding(2) var gBufferDepth: texture_depth_2d;

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
	let pixel = vec2i(floor(coord.xy));
	let dims = vec2f(textureDimensions(gBufferDepth));
	let c = coord.xy / dims;

	if (c.x < 0.33333) {
		let rawDepth = textureLoad(gBufferDepth, pixel, 0);
		let depth = (1.0 - rawDepth) * 50.0;
		return vec4f(depth, depth, depth, 1.0);
	}
	if (c.x < 0.66667) {
		let normal = textureLoad(gBufferNormal, pixel, 0).xyz;
		return vec4f(normal * 0.5 + vec3f(0.5), 1.0);
	}
	return textureLoad(gBufferAlbedo, pixel, 0);
}
