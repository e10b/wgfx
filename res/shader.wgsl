@group(0) @binding(0) var<uniform> camera: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;

@group(0) @binding(2) var gradientTexture: texture_2d<f32>;
@group(0) @binding(3) var textureSampler: sampler;

@group(0) @binding(4) var<uniform> fogAmount: f32;
@group(0) @binding(5) var<uniform> camPos: vec4f;

@group(0) @binding(6) var<uniform> view: mat4x4f;
@group(0) @binding(7) var<uniform> proj: mat4x4f;


struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) uv: vec2f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,
	@location(1) uv: vec2f,
	@location(2) worldPos: vec3f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	

	let world = model * vec4f(in.position, 1.0);
	out.worldPos = world.xyz;
	
	
	let viewPos = view * world;
	
	//out.position = camera * world;
	out.position = proj * viewPos;

	//out.position = camera * vec4f(in.position, 1.0);
	//out.color = in.color;
	//out.worldPos = (model *  vec4f(in.position, 1.0)).xyz;


	out.normal = in.normal;

	out.uv = in.uv;			//slight offset for graphical glitch -- * .99 might be necessary -- maybe not
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));

    let sunlightDirection = normalize(vec3f(0.3, 0.5, -1.0));  // Adjust direction for desired angle
    let sunlightColor = vec3f(2.0, 1.85, 1.6);  // Warm sunlight color

    let ambientColor = vec3f(0.4, 0.4, 0.6);  // Soft, cool ambient light

    let normal = normalize(in.normal);
    let sunlightShading = max(0.0, dot(sunlightDirection, normal));
    
    let shading = sunlightShading * sunlightColor + ambientColor;

    let color = textureSample(gradientTexture, textureSampler, in.uv).rgb * shading;

		//viewvec
		var view = in.worldPos - camPos.xyz;
		let viewDist = length(view);
		view /= viewDist;


	let FOG_COLOR = vec3(0.4, 0.7, 1.0);
	let LIGHT_DIR = vec3(0.5, 1.0, -0.7);
	let SUN_COLOR = vec3(1.0, 0.8, 0.4);

	let falloff = 0.02;
	var fogAmount = 0.5 * exp(-camPos.y * falloff) * (1.0 - exp(-viewDist * view.y * falloff)) / view.y;
	fogAmount = clamp(fogAmount, 0.0, 1.0);
	let fogColor = mix(vec3(FOG_COLOR), vec3(1.0, 0.9, 0.7), 0.5 * pow(max(0.0, dot(view, vec3(LIGHT_DIR))), 2.0));

	var c = vec3(SUN_COLOR * color);
	c = mix(color, fogColor, fogAmount);


    let corrected_color = pow(c, vec3f(2.2));
    return vec4f(corrected_color, 1.0);
}