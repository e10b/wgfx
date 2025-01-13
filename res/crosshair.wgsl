@group(0) @binding(0) var<uniform> size: vec2f;

struct VertexInput {
	@location(0) position: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	
	out.position = vec4(in.position.xy * size, in.position.z, 1.0f);

	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    
    return vec4(1.0f);

}