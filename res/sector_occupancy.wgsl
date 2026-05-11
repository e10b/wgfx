@group(0) @binding(4) var topLevelGridTex: texture_3d<u32>;
@group(0) @binding(10) var<storage, read_write> sectorMap: array<u32>;

const TOP_LEVEL_SIZE: i32 = 256;
const BRICKS_PER_SECTOR: i32 = 4;
const SECTORS_PER_DIM: i32 = TOP_LEVEL_SIZE / BRICKS_PER_SECTOR;

var<workgroup> sectorOccupied: atomic<u32>;

@compute @workgroup_size(4, 4, 4)
fn main(
    @builtin(workgroup_id) workgroupId: vec3<u32>,
    @builtin(local_invocation_id) localId: vec3<u32>,
    @builtin(local_invocation_index) localIndex: u32
) {
    let sectorPos = vec3<i32>(workgroupId);

    if (sectorPos.x >= SECTORS_PER_DIM ||
        sectorPos.y >= SECTORS_PER_DIM ||
        sectorPos.z >= SECTORS_PER_DIM) {
        return;
    }

    if (localIndex == 0u) {
        atomicStore(&sectorOccupied, 0u);
    }
    workgroupBarrier();

    let topPos = sectorPos * BRICKS_PER_SECTOR + vec3<i32>(localId);
    let topEntry = textureLoad(topLevelGridTex, topPos, 0).r;
    if (topEntry != 0u) {
        atomicStore(&sectorOccupied, 1u);
    }
    workgroupBarrier();

    if (localIndex == 0u) {
        let sectorIndex = u32(
            sectorPos.x +
            sectorPos.y * SECTORS_PER_DIM +
            sectorPos.z * SECTORS_PER_DIM * SECTORS_PER_DIM
        );
        sectorMap[sectorIndex] = atomicLoad(&sectorOccupied);
    }
}
