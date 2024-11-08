@group(0) @binding(0) var<uniform> camera: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;

@group(0) @binding(2) var gradientTexture: texture_2d<f32>;
@group(0) @binding(3) var textureSampler: sampler;

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
	
	out.position = camera * model * vec4f(in.position, 1.0);
	//out.color = in.color;

	out.normal = in.normal;

	out.uv = in.uv;			//slight offset for graphical glitch -- * .99 might be necessary -- maybe not
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));

    // Define the sunlight direction and color
    let sunlightDirection = normalize(vec3f(0.3, 0.5, -1.0));  // Adjust direction for desired angle
    let sunlightColor = vec3f(1.0, 0.85, 0.6);  // Warm sunlight color

    // Define a subtle ambient color to simulate sky light
    let ambientColor = vec3f(0.1, 0.1, 0.2);  // Soft, cool ambient light

    // Calculate the light shading based on sunlight direction
    let normal = normalize(in.normal);
    let sunlightShading = max(0.0, dot(sunlightDirection, normal));
    
    // Combine sunlight and ambient lighting
    let shading = sunlightShading * sunlightColor + ambientColor;

    // Sample texture color and apply shading
    let color = textureSample(gradientTexture, textureSampler, in.uv).rgb * shading;

    // Apply gamma correction
    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1.0);
}