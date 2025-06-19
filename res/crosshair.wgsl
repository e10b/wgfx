@group(0) @binding(0) var<uniform> size: vec2f;
@group(0) @binding(1) var tex: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f, // This must match the fragment input location
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4f(input.position, 1.0);
    output.uv = input.uv;
    return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    return textureSample(tex, texSampler, input.uv);
    //return vec4f(.2, .2, .2, 1.0);
    //return vec4<f32>(input.uv, 0.0, 1.0);
}