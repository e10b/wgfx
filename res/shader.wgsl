@group(0) @binding(0) var<uniform> camera: mat4x4f;

@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
@group(0) @binding(2) var textureSampler: sampler;

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
	
	out.position = camera * vec4f(in.position, 1.0);
	//out.color = in.color;

	out.normal = in.normal;

	out.uv = in.uv;			//slight offset for graphical glitch -- * .99 might be necessary -- maybe not
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));
	
	let normal = normalize(in.normal);

	//let lightColor1 = vec3f(1.0, 0.9, 0.6);
	//let lightColor2 = vec3f(0.6, 0.9, 1.0);
	//let lightDirection1 = vec3f(0.5, 0.9, -0.4);
	//let lightDirection2 = vec3f(0.2, -0.4, -0.5);
	//let shading1 = max(0.0, dot(lightDirection1, normal));
	//let shading2 = max(0.0, dot(lightDirection2, normal));
	//let shading = shading1 * lightColor1 + shading2 * lightColor2;

	let shading = max(0.0, dot(vec3(0.5, 0.9, -0.4), normal)) * vec3(1.0, 1.0, 1.0);

		//let color = textureLoad(gradientTexture, texelCoords, 0).rgb;
	let color = textureSample(gradientTexture, textureSampler, in.uv).rgb;//* shading;
	//let color = vec3f(normal); // This will give a gradient based on UVs



	let corrected_color = pow(color, vec3f(2.2));

	return vec4f(corrected_color, 1.0);
}