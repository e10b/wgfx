@group(0) @binding(0) var myTex : texture_2d<f32>;
@group(0) @binding(1) var mySamp : sampler;
@group(0) @binding(2) var<storage, read> voxelBuffer0 : array<u32>;
@group(0) @binding(3) var<storage, read> voxelBuffer1 : array<u32>;
@group(0) @binding(4) var<storage, read> voxelBuffer2 : array<u32>;
@group(0) @binding(5) var<storage, read> voxelBuffer3 : array<u32>;

struct VertexInput {
    @location(0) position: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output : VertexOutput;
    output.position = vec4f(input.position, 1.0);
    output.uv = (input.position.xy + 1.0) * 0.5;
    output.uv.y = 1.0 - output.uv.y;
    return output;
}

// Hierarchy: Sector → Brick → Voxel
const BRICK_SIZE = 8u;
const BRICKS_PER_SECTOR = 2u; // 2x2x2 bricks per sector
const SECTOR_SIZE = BRICK_SIZE * BRICKS_PER_SECTOR; // 16 voxels per sector
const GRID_SIZE = 512u;
const NUM_SECTORS_X = i32(GRID_SIZE / SECTOR_SIZE); // 32 sectors
const NUM_SECTORS_Y = i32(GRID_SIZE / SECTOR_SIZE); // 32 sectors
const NUM_SECTORS_Z = i32(GRID_SIZE / SECTOR_SIZE); // 32 sectors
const BRICKS_X = i32(GRID_SIZE / BRICK_SIZE); // 64 bricks
const BRICKS_Y = i32(GRID_SIZE / BRICK_SIZE); // 64 bricks
const BRICKS_Z = i32(GRID_SIZE / BRICK_SIZE); // 64 bricks

// Helper function to read voxel from split buffers (total 4GB split into four 1GB buffers)
fn getVoxelFromBuffers(index: u32) -> u32 {
    let bufferSize = 1073741824u / 4u; // 1GB / 4 bytes per uint32
    if (index < bufferSize) {
        return voxelBuffer0[index];
    } else if (index < bufferSize * 2u) {
        return voxelBuffer1[index - bufferSize];
    } else if (index < bufferSize * 3u) {
        return voxelBuffer2[index - bufferSize * 2u];
    } else {
        return voxelBuffer3[index - bufferSize * 3u];
    }
}

// Convert voxel coordinates to sector coordinates
fn getVoxelSector(voxelPos: vec3<i32>) -> vec3<i32> {
    return voxelPos / i32(SECTOR_SIZE);
}
// Convert voxel coordinates to brick coordinates
fn getVoxelBrick(voxelPos: vec3<i32>) -> vec3<i32> {
    return voxelPos / i32(BRICK_SIZE);
}
// Convert voxel coordinates to local brick coordinates (0-7 within brick)
fn getVoxelLocalCoord(voxelPos: vec3<i32>) -> vec3<i32> {
    return voxelPos % i32(BRICK_SIZE);
}
// Convert voxel coordinates to local sector coordinates (0-15 within sector)
fn getVoxelLocalSectorCoord(voxelPos: vec3<i32>) -> vec3<i32> {
    return voxelPos % i32(SECTOR_SIZE);
}
// Check if a brick contains any solid voxels
fn isBrickOccupied(brickPos: vec3<i32>) -> bool {
    // Check bounds
    if (brickPos.x < 0 || brickPos.x >= BRICKS_X) { return false; }
    if (brickPos.y < 0 || brickPos.y >= BRICKS_Y) { return false; }
    if (brickPos.z < 0 || brickPos.z >= BRICKS_Z) { return false; }
    
    for (var x: i32 = 0; x < i32(BRICK_SIZE); x = x + 1) {
        for (var y: i32 = 0; y < i32(BRICK_SIZE); y = y + 1) {
            for (var z: i32 = 0; z < i32(BRICK_SIZE); z = z + 1) {
                let voxelPos = brickPos * i32(BRICK_SIZE) + vec3<i32>(x, y, z);
                let index = u32(voxelPos.x + voxelPos.y * i32(GRID_SIZE) + voxelPos.z * i32(GRID_SIZE) * i32(GRID_SIZE));
                if (getVoxelFromBuffers(index) != 0u) { return true; }
            }
        }
    }
    return false;
}

// Get brick coordinates from voxel coordinates
fn getBrickCoords(voxelPos: vec3<i32>) -> vec3<i32> {
    return vec3<i32>(
        voxelPos.x / i32(BRICK_SIZE),
        voxelPos.y / i32(BRICK_SIZE),
        voxelPos.z / i32(BRICK_SIZE)
    );
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
    var color = textureSample(myTex, mySamp, input.uv);
    
    // Apply gamma correction (Emscripten WebGPU canvas defaults to linear)
    let gamma = 1.0 / 2.2;
    color = vec4<f32>(pow(color.rgb, vec3<f32>(gamma)), color.a);
    
    return color;
}