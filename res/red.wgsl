@group(0) @binding(0) var<uniform> size: vec2f;

struct VertexInput {
    @location(0) position: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
};

@vertex 
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4f(input.position, 1.0);
    return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    return vec4f(sin(size.x * 3000 + 0.1)+0.4, sin(size.x * 4000 + 0.1), sin(size.x * 5100 + 0.1), 1.0);
}