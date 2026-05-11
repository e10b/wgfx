@group(0) @binding(0) var<uniform> cameraPos: vec4<f32>;
@group(0) @binding(1) var outTex: texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(2) var<uniform> invViewProj: mat4x4<f32>;
@group(0) @binding(3) var<uniform> res: vec4<f32>;

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
	if (global_id.x >= u32(res.x) || global_id.y >= u32(res.y)) {
		return;
	}

	let pixel = vec2<f32>(vec2<u32>(global_id.xy)) + vec2<f32>(0.5, 0.5);
	let uv = pixel / res.xy;
	let p = (uv - vec2<f32>(0.5, 0.5)) * vec2<f32>(res.x / res.y, 1.0);
	let radius = 0.34;
	let dist2 = dot(p, p);

	var color = vec3<f32>(0.06, 0.08, 0.12);
	if (dist2 <= radius * radius) {
		let z = sqrt(radius * radius - dist2);
		let normal = normalize(vec3<f32>(p / radius, z / radius));
		let lightDir = normalize(vec3<f32>(-0.4, 0.8, 0.6));
		let diffuse = max(dot(normal, lightDir), 0.0);
		let ambient = 0.2;
		let baseColor = vec3<f32>(0.95, 0.55, 0.35);
		let rim = pow(1.0 - max(normal.z, 0.0), 2.5) * 0.18;
		color = baseColor * (ambient + diffuse * 0.85) + rim;
	}

	textureStore(outTex, vec2<i32>(global_id.xy), vec4<f32>(color, 1.0));
}