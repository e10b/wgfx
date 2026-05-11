struct VoxelEditCommand {
    wordIndex: u32,
    wordValue: u32,
    _pad: u32,
    _pad2: u32,
};

// Voxel structure: 2 bytes total (int8 density + uint8 material)
struct Voxel {
    density: i32,    // Density value (-128 to 127)
    material: u32,   // Material/color palette index (0-255)
}

// Pack Voxel struct into u32
fn packVoxel(v: Voxel) -> u32 {
    let d = u32(v.density) & 0xFFu;
    let m = v.material & 0xFFu;
    return (m << 8u) | d;
}

// Unpack u32 into Voxel struct
fn unpackVoxel(data: u32) -> Voxel {
    let d = i32(data & 0xFFu);
    // Convert unsigned byte to signed (handle 2's complement)
    let density = select(d - 256, d, d < 128);
    let material = (data >> 8u) & 0xFFu;
    return Voxel(density, material);
}

@group(0) @binding(6) var<storage, read_write> brickPool: array<u32>;
@group(0) @binding(11) var<storage, read> editCount: array<u32>;
@group(0) @binding(12) var<storage, read> editCommands: array<VoxelEditCommand>;

@compute @workgroup_size(64, 1, 1)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let count = editCount[0];
    let idx = gid.x;
    if (idx >= count) {
        return;
    }

    let cmd = editCommands[idx];
    brickPool[cmd.wordIndex] = cmd.wordValue;
}
