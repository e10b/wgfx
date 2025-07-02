@group(0) @binding(0) var<uniform> view: mat4x4f;
@group(0) @binding(1) var<uniform> model: mat4x4f;
@group(0) @binding(2) var<uniform> proj: mat4x4f;

@group(0) @binding(3) var gradientTexture: texture_2d<f32>;
@group(0) @binding(4) var textureSampler: sampler;

@group(0) @binding(5) var tex: texture_depth_2d;
@group(0) @binding(6) var texSampler: sampler;
@group(0) @binding(7) var<uniform> lightView: mat4x4f;


struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) uv: vec2f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,
	@location(1) uv: vec2f,
	@location(2) worldPos: vec3f,
};

struct FragmentOutput {
    @location(0) color0: vec4f,
    @location(1) color1: vec4f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	

	let worldPos = model * vec4f(in.position, 1.0);


	out.position = proj * view * model * vec4f(in.position, 1.0);
	//out.color = in.color;
	    out.normal = (model * vec4f(in.normal, 0.0)).xyz;

	out.uv = in.uv;			//slight offset for graphical glitch -- * .99 might be necessary -- maybe not
	out.worldPos = worldPos.xyz;
	
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    // Normalize normal
    let normal = normalize(in.normal);

    // Light setup
    let lightColor1 = vec3f(1.0, 0.9, 0.6);
    let lightColor2 = vec3f(0.6, 0.9, 1.0);
    let lightDir1 = normalize(vec3f(0.5, 0.9, -0.4));
    let lightDir2 = normalize(vec3f(0.2, -0.4, -0.5));
    let shading1 = max(dot(lightDir1, normal), 0.0);
    let shading2 = max(dot(lightDir2, normal), 0.0);
    let shading = shading1 * lightColor1 + shading2 * lightColor2;

    // Compute light-space clip position
    let lightClip = proj * lightView * vec4f(in.worldPos, 1.0);
    let lightNDC = lightClip.xyz / lightClip.w;

    // Convert to UV coordinates in [0,1] space
    let shadowUV = clamp(lightNDC.xy * 0.5 + vec2f(0.5), vec2f(0.0), vec2f(1.0));

    // Get current fragment depth in light space
    let shadowDepth = lightNDC.z * 0.5 + 0.5;

    // Sample shadow map depth
    let mapDepth = textureSample(tex, texSampler, shadowUV);

    // Compare: is fragment in shadow?
    let bias = 0.005;
    let shadow = select(0.3, 1.0, shadowDepth <= mapDepth + bias);

    // Sample color
    let albedo = pow(textureSample(gradientTexture, textureSampler, in.uv).rgb, vec3f(2.2));

    // Apply shading and shadow
    let finalColor = vec4f(albedo * shading * shadow, 1.0);

    // Output
    var out: FragmentOutput;
    out.color0 = finalColor;
    out.color1 = finalColor;
    return out;
}
