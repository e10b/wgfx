
@group(0) @binding(0) var<uniform> position: vec3<f32>;
@group(0) @binding(1) var outTex : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(2) var<uniform> invViewProj: mat4x4<f32>;
@group(0) @binding(3) var<uniform> res: vec4<f32>;
@group(0) @binding(4) var topLevelGridTex: texture_3d<u32>;
@group(0) @binding(5) var topLevelSampler: sampler;
@group(0) @binding(6) var<storage, read_write> brickPool: array<u32>;
@group(0) @binding(7) var<storage, read_write> colorPalette: array<u32, 256>; // 256-color palette

@group(0) @binding(8) var owlTexture: texture_2d<f32>;
@group(0) @binding(9) var owlSampler: sampler;
@group(0) @binding(10) var<storage, read_write> sectorMap: array<u32>;

// Hierarchical brickmap constants (all powers of 2 for efficient operations)
const TOP_LEVEL_SIZE = 256; // 256³ top-level grid (power of 2)
const BRICK_SIZE = 8; // 8³ bricks (power of 2)
const BRICKS_PER_SECTOR = 4; // 4³ bricks per sector
const SECTOR_SIZE = BRICK_SIZE * BRICKS_PER_SECTOR; // 32 voxels per sector
const SECTORS_PER_DIM = TOP_LEVEL_SIZE / BRICKS_PER_SECTOR; // 16 sectors per dimension
const WORLD_SIZE = TOP_LEVEL_SIZE * BRICK_SIZE; // 512³ world
const MAX_BRICK_POOL_SIZE = 524288u; // 512K bricks (power of 2)

// Bit manipulation constants
const BRICK_INDEX_MASK = 0x7FFFFFFFu; // Bits 0-30
const IS_BRICK_FLAG = 0x80000000u;    // Bit 31

// Voxel structure: 2 bytes total (int8 density + uint8 material)
// Stored in u32 brickPool as: density in bits 0-7, material in bits 8-15
struct Voxel {
    density: i32,    // Density value (-128 to 127), stored as i32 but represents int8
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

// Check if voxel is empty
fn isVoxelEmpty(v: Voxel) -> bool {
    return v.density == 0 && v.material == 0u;
}

// Get top-level grid index from world coordinates
fn getTopLevelIndex(pos: vec3<i32>) -> u32 {
    let topX = pos.x / BRICK_SIZE;
    let topY = pos.y / BRICK_SIZE;
    let topZ = pos.z / BRICK_SIZE;
    return u32(topX + topY * TOP_LEVEL_SIZE + topZ * TOP_LEVEL_SIZE * TOP_LEVEL_SIZE);
}

// Sample from the 3D texture using integer coordinates
fn sampleTopLevelGrid(topPos: vec3<i32>) -> u32 {
    // Convert to texture coordinates (0.0 to 1.0 range)
    let texCoords = (vec3<f32>(topPos) + 0.5) / f32(TOP_LEVEL_SIZE);
    
    // Sample the 3D texture - textureLoad is better for exact integer lookups
    let sample = textureLoad(topLevelGridTex, topPos, 0);
    return sample.r;
}

// Get top-level grid entry from world coordinates  
fn getTopLevelEntry(pos: vec3<i32>) -> u32 {
    let topX = pos.x / BRICK_SIZE;
    let topY = pos.y / BRICK_SIZE;
    let topZ = pos.z / BRICK_SIZE;
    
    // Check bounds
    if (topX < 0 || topX >= TOP_LEVEL_SIZE ||
        topY < 0 || topY >= TOP_LEVEL_SIZE ||
        topZ < 0 || topZ >= TOP_LEVEL_SIZE) {
        return 0u;
    }
    
    return sampleTopLevelGrid(vec3<i32>(topX, topY, topZ));
}

// Check if a top-level grid cell contains any solid voxels
fn isTopLevelOccupied(topPos: vec3<i32>) -> bool {
    // Check bounds
    if (topPos.x < 0 || topPos.x >= TOP_LEVEL_SIZE ||
        topPos.y < 0 || topPos.y >= TOP_LEVEL_SIZE ||
        topPos.z < 0 || topPos.z >= TOP_LEVEL_SIZE) {
        return false;
    }

    let topEntry = sampleTopLevelGrid(topPos);
    
    // If it's a solid color, check if it's non-zero
    if ((topEntry & IS_BRICK_FLAG) == 0u) {
        return topEntry != 0u;
    }
    
    // If it's a brick, we need to check if it has any solid voxels
    // For now, assume if it's a brick, it has some content
    return true;
}

// Get top-level coordinates from voxel coordinates
fn getTopLevelCoords(voxelPos: vec3<i32>) -> vec3<i32> {
    return vec3<i32>(
        voxelPos.x / BRICK_SIZE,
        voxelPos.y / BRICK_SIZE,
        voxelPos.z / BRICK_SIZE
    );
}
// Get sector coordinates from world coordinates
fn getSectorCoords(voxelPos: vec3<i32>) -> vec3<i32> {
    return vec3<i32>(
        voxelPos.x / SECTOR_SIZE,
        voxelPos.y / SECTOR_SIZE,
        voxelPos.z / SECTOR_SIZE
    );
}

// Check if a sector contains any solid voxels
fn isSectorOccupied(sectorPos: vec3<i32>) -> bool {
    if (sectorPos.x < 0 || sectorPos.x >= SECTORS_PER_DIM ||
        sectorPos.y < 0 || sectorPos.y >= SECTORS_PER_DIM ||
        sectorPos.z < 0 || sectorPos.z >= SECTORS_PER_DIM) {
        return false;
    }

    let sectorIndex = u32(sectorPos.x + sectorPos.y * SECTORS_PER_DIM + sectorPos.z * SECTORS_PER_DIM * SECTORS_PER_DIM);
    return sectorMap[sectorIndex] != 0u;
}
struct DDAHit {
t: f32,
voxel : vec3<i32>,
normal : vec3<f32>,
steps : i32,
};

// Hierarchical DDA that uses top-level grid to skip empty regions
// Enhanced DDA with camera precision fix
fn hierarchicalVoxelDDA(
    rayOrigin: vec3<f32>,
    rayDir : vec3<f32>,
    gridMin : vec3<i32>,
    gridMax : vec3<i32>,
    cameraPos: vec3<f32>,
    cameraPosInt: vec3<i32>,
    cameraPosOffset: vec3<f32>
) -> DDAHit {
    let gridMinRelative = vec3<f32>(gridMin - cameraPosInt) - cameraPosOffset;
    let gridMaxRelative = vec3<f32>(gridMax - cameraPosInt) - cameraPosOffset;
    let gridMinF = gridMinRelative;
    let gridMaxF = gridMaxRelative + vec3<f32>(1.0, 1.0, 1.0);

    // === 1. Intersect the grid bounding box ===
    var tMin = -1e10;
    var tMax = 1e10;

    for (var i = 0u; i < 3u; i = i + 1u) {
        let epsilon = 1e-8;
        let safeRayDir = select(rayDir[i], epsilon, abs(rayDir[i]) < epsilon);
        let invD = 1.0 / safeRayDir;

        var t0 = (gridMinF[i] - rayOrigin[i]) * invD;
        var t1 = (gridMaxF[i] - rayOrigin[i]) * invD;
        if (invD < 0.0) {
            let tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        tMin = max(tMin, t0);
        tMax = min(tMax, t1);
    }

    if (tMax < tMin) {
        return DDAHit(-1.0, vec3<i32>(0), vec3<f32>(0), -1);
    }

    // === 2. Start at entry point ===
    var t = max(tMin, 0.0);
    if (t == tMin) {
        t += 1e-4; // Larger epsilon for better precision
    }
    var p = rayOrigin + t * rayDir;
    
    // PRECISION FIX: Calculate position relative to camera's integer voxel position
    let pWithOffset = p + cameraPosOffset;

    // Use double precision for voxel calculation to reduce jitter
    var voxel = vec3<i32>(
        clamp(i32(floor(pWithOffset.x + 0.5 * sign(rayDir.x) * 1e-6)) + cameraPosInt.x, gridMin.x, gridMax.x),
        clamp(i32(floor(pWithOffset.y + 0.5 * sign(rayDir.y) * 1e-6)) + cameraPosInt.y, gridMin.y, gridMax.y),
        clamp(i32(floor(pWithOffset.z + 0.5 * sign(rayDir.z) * 1e-6)) + cameraPosInt.z, gridMin.z, gridMax.z)
    );

    let step = vec3<i32>(
        select(-1, 1, rayDir.x >= 0.0),
        select(-1, 1, rayDir.y >= 0.0),
        select(-1, 1, rayDir.z >= 0.0)
    );

    let epsilon = 1e-6; // Reduced epsilon for better precision
    let safeRayDir = vec3<f32>(
        select(rayDir.x, epsilon, abs(rayDir.x) < epsilon),
        select(rayDir.y, epsilon, abs(rayDir.y) < epsilon),
        select(rayDir.z, epsilon, abs(rayDir.z) < epsilon)
    );
    let invDir = 1.0 / safeRayDir;

    // Pre-calculate DDA increments for efficiency with better precision
    var next = vec3<f32>(0.0);
    var delta = vec3<f32>(0.0);
    for (var i = 0u; i < 3u; i = i + 1u) {
        let voxelBoundary = f32(voxel[i]) + select(0.0, 1.0, step[i] > 0);
        let boundaryOffsetFromCameraInt = voxelBoundary - f32(cameraPosInt[i]);
        let boundaryRelativeToRayOrigin = boundaryOffsetFromCameraInt - cameraPosOffset[i];
        next[i] = (boundaryRelativeToRayOrigin - rayOrigin[i]) * invDir[i];
        delta[i] = abs(invDir[i]);
    }

    var normal = vec3<f32>(0.0);
    let maxSteps = 512;
    var stepCount = 0;
    var currentTopLevel = vec3<i32>(-1000); // Invalid to trigger first-enter evaluation
    var currentSector = vec3<i32>(-1000); // Invalid to trigger first-enter evaluation

    for (var i = 0; i < maxSteps; i = i + 1) {
        stepCount = i;

        // Check if we're still in bounds
        if (all(voxel >= gridMin) && all(voxel <= gridMax)) {
            let newSector = getSectorCoords(voxel);
            if (any(newSector != currentSector)) {
                currentSector = newSector;
                if (!isSectorOccupied(currentSector)) {
                    let sectorMin = vec3<f32>(
                        f32(currentSector.x * SECTOR_SIZE - cameraPosInt.x) - cameraPosOffset.x,
                        f32(currentSector.y * SECTOR_SIZE - cameraPosInt.y) - cameraPosOffset.y,
                        f32(currentSector.z * SECTOR_SIZE - cameraPosInt.z) - cameraPosOffset.z);
                    let sectorMax = vec3<f32>(
                        f32((currentSector.x + 1) * SECTOR_SIZE - cameraPosInt.x) - cameraPosOffset.x,
                        f32((currentSector.y + 1) * SECTOR_SIZE - cameraPosInt.y) - cameraPosOffset.y,
                        f32((currentSector.z + 1) * SECTOR_SIZE - cameraPosInt.z) - cameraPosOffset.z);

                    var sectorExit = vec3<f32>(1e10);
                    for (var j = 0u; j < 3u; j = j + 1u) {
                        if (abs(rayDir[j]) > 1e-8) {
                            let boundary = select(sectorMin[j], sectorMax[j], rayDir[j] > 0.0);
                            sectorExit[j] = (boundary - rayOrigin[j]) / rayDir[j];
                        }
                    }

                    let exitT = min(sectorExit.x, min(sectorExit.y, sectorExit.z));
                    if (exitT > t && exitT <= tMax) {
                        t = exitT + 1e-5;
                        p = rayOrigin + t * rayDir;
                        let pWithOffset2 = p + cameraPosOffset;
                        
                        voxel = vec3<i32>(
                            clamp(i32(floor(pWithOffset2.x + sign(rayDir.x) * 0.001)) + cameraPosInt.x, gridMin.x, gridMax.x),
                            clamp(i32(floor(pWithOffset2.y + sign(rayDir.y) * 0.001)) + cameraPosInt.y, gridMin.y, gridMax.y),
                            clamp(i32(floor(pWithOffset2.z + sign(rayDir.z) * 0.001)) + cameraPosInt.z, gridMin.z, gridMax.z));

                        // Determine which face we exited from and safely bound voxel mathematically
                        if (abs(exitT - sectorExit.x) < abs(exitT - sectorExit.y) &&
                            abs(exitT - sectorExit.x) < abs(exitT - sectorExit.z)) {
                            voxel.x = currentSector.x * SECTOR_SIZE + select(0, SECTOR_SIZE, step.x > 0);
                            if (step.x < 0) { voxel.x -= 1; }
                            normal = vec3<f32>(-f32(step.x), 0.0, 0.0);
                        }
                        else if (abs(exitT - sectorExit.y) < abs(exitT - sectorExit.z)) {
                            voxel.y = currentSector.y * SECTOR_SIZE + select(0, SECTOR_SIZE, step.y > 0);
                            if (step.y < 0) { voxel.y -= 1; }
                            normal = vec3<f32>(0.0, -f32(step.y), 0.0);
                        }
                        else {
                            voxel.z = currentSector.z * SECTOR_SIZE + select(0, SECTOR_SIZE, step.z > 0);
                            if (step.z < 0) { voxel.z -= 1; }
                            normal = vec3<f32>(0.0, 0.0, -f32(step.z));
                        }

                        // Recalculate DDA boundaries perfectly aligned
                        for (var j = 0u; j < 3u; j = j + 1u) {
                            let voxelBoundary2 = f32(voxel[j]) + select(0.0, 1.0, step[j] > 0);
                            let boundaryOffsetFromCameraInt2 = voxelBoundary2 - f32(cameraPosInt[j]);
                            let boundaryRelativeToRayOrigin2 = boundaryOffsetFromCameraInt2 - cameraPosOffset[j];
                            next[j] = (boundaryRelativeToRayOrigin2 - rayOrigin[j]) * invDir[j];
                        }
                        continue;
                    }
                }
            }

            let newTopLevel = getTopLevelCoords(voxel);

            // If we've moved to a new top-level cell, check if it's occupied
            if (any(newTopLevel != currentTopLevel)) {
                currentTopLevel = newTopLevel;

                // If top-level cell is not occupied, skip directly to top-level boundary
                if (!isTopLevelOccupied(currentTopLevel)) {
                    // Calculate the top-level cell boundaries
                    let topMin = vec3<f32>(
                        f32(currentTopLevel.x * BRICK_SIZE - cameraPosInt.x) - cameraPosOffset.x,
                        f32(currentTopLevel.y * BRICK_SIZE - cameraPosInt.y) - cameraPosOffset.y,
                        f32(currentTopLevel.z * BRICK_SIZE - cameraPosInt.z) - cameraPosOffset.z);
                    let topMax = vec3<f32>(
                        f32((currentTopLevel.x + 1) * BRICK_SIZE - cameraPosInt.x) - cameraPosOffset.x,
                        f32((currentTopLevel.y + 1) * BRICK_SIZE - cameraPosInt.y) - cameraPosOffset.y,
                        f32((currentTopLevel.z + 1) * BRICK_SIZE - cameraPosInt.z) - cameraPosOffset.z);

                    // Calculate intersection with top-level boundaries
                    var topExit = vec3<f32>(1e10);
                    for (var j = 0u; j < 3u; j = j + 1u) {
                        if (abs(rayDir[j]) > 1e-8) {
                            let boundary = select(topMin[j], topMax[j], rayDir[j] > 0.0);
                            topExit[j] = (boundary - rayOrigin[j]) / rayDir[j];
                        }
                    }

                    let exitT = min(topExit.x, min(topExit.y, topExit.z));

                    if (exitT > t && exitT <= tMax) {
                        t = exitT + 1e-5;
                        p = rayOrigin + t * rayDir;
                        let pWithOffset3 = p + cameraPosOffset;

                        voxel = vec3<i32>(
                            clamp(i32(floor(pWithOffset3.x + sign(rayDir.x) * 0.001)) + cameraPosInt.x, gridMin.x, gridMax.x),
                            clamp(i32(floor(pWithOffset3.y + sign(rayDir.y) * 0.001)) + cameraPosInt.y, gridMin.y, gridMax.y),
                            clamp(i32(floor(pWithOffset3.z + sign(rayDir.z) * 0.001)) + cameraPosInt.z, gridMin.z, gridMax.z)
                        );

                        // Determine which face we exited from to force integer boundary snap safely
                        if (abs(exitT - topExit.x) < abs(exitT - topExit.y) &&
                            abs(exitT - topExit.x) < abs(exitT - topExit.z)) {
                            voxel.x = currentTopLevel.x * BRICK_SIZE + select(0, BRICK_SIZE, step.x > 0);
                            if (step.x < 0) { voxel.x -= 1; }
                            normal = vec3<f32>(-f32(step.x), 0.0, 0.0);
                        }
                        else if (abs(exitT - topExit.y) < abs(exitT - topExit.z)) {
                            voxel.y = currentTopLevel.y * BRICK_SIZE + select(0, BRICK_SIZE, step.y > 0);
                            if (step.y < 0) { voxel.y -= 1; }
                            normal = vec3<f32>(0.0, -f32(step.y), 0.0);
                        }
                        else {
                            voxel.z = currentTopLevel.z * BRICK_SIZE + select(0, BRICK_SIZE, step.z > 0);
                            if (step.z < 0) { voxel.z -= 1; }
                            normal = vec3<f32>(0.0, 0.0, -f32(step.z));
                        }
                        
                        // Recalculate DDA bounds perfectly
                        for (var j = 0u; j < 3u; j = j + 1u) {
                            let voxelBoundary3 = f32(voxel[j]) + select(0.0, 1.0, step[j] > 0);
                            let boundaryOffsetFromCameraInt3 = voxelBoundary3 - f32(cameraPosInt[j]);
                            let boundaryRelativeToRayOrigin3 = boundaryOffsetFromCameraInt3 - cameraPosOffset[j];
                            next[j] = (boundaryRelativeToRayOrigin3 - rayOrigin[j]) * invDir[j];
                        }

                        continue; // Skip to next iteration
                    }
                }
            }

            // If we're in an occupied top-level cell, check the actual voxel
            if (isVoxelSolid(voxel)) {
                return DDAHit(t, voxel, normal, i);
            }
        }

        // Standard DDA step - use consistent epsilon to prevent jitter
        let eps = 1e-5; // Larger epsilon for better stability
        if (next.x <= next.y + eps && next.x <= next.z + eps) {
            voxel.x += step.x;
            t = next.x;
            next.x += delta.x;
            normal = vec3<f32>(-f32(step.x), 0.0, 0.0);
        }
        else if (next.y <= next.z + eps) {
            voxel.y += step.y;
            t = next.y;
            next.y += delta.y;
            normal = vec3<f32>(0.0, -f32(step.y), 0.0);
        }
        else {
            voxel.z += step.z;
            t = next.z;
            next.z += delta.z;
            normal = vec3<f32>(0.0, 0.0, -f32(step.z));
        }

        if (t > tMax) {
            break;
        }
    }

    return DDAHit(-1.0, vec3<i32>(0), vec3<f32>(0), stepCount);
}

// Function to check if a voxel is solid based on the hierarchical storage
fn isVoxelSolid(voxelPos: vec3<i32>) -> bool {
    // Check bounds
    if (voxelPos.x < 0 || voxelPos.x >= WORLD_SIZE ||
        voxelPos.y < 0 || voxelPos.y >= WORLD_SIZE ||
        voxelPos.z < 0 || voxelPos.z >= WORLD_SIZE) {
        return false;
    }

    // Get top-level grid entry using the new texture-based method
    let topEntry = getTopLevelEntry(voxelPos);

    if ((topEntry & IS_BRICK_FLAG) == 0u) {
        // It's a solid color
        return topEntry != 0u;
    } else {
        // It's a brick - check individual voxel
        let brickIndex = topEntry & BRICK_INDEX_MASK;
        
        // Bounds check for brick index
        if (brickIndex >= MAX_BRICK_POOL_SIZE) {
            return false;
        }
        
        let localX = voxelPos.x % BRICK_SIZE;
        let localY = voxelPos.y % BRICK_SIZE;
        let localZ = voxelPos.z % BRICK_SIZE;
        
        // Each brick has 512 uint16 voxels, packed 2 per u32 (= 256 u32 per brick)
        let voxelIndex = brickIndex * 512u + u32(localX + localY * BRICK_SIZE + localZ * BRICK_SIZE * BRICK_SIZE);
        let wordIndex = voxelIndex / 2u;   // 2 uint16s per u32
        let halfIndex = voxelIndex % 2u;   // which uint16 within the u32 (0=low, 1=high)
        
        let word = brickPool[wordIndex];
        let voxelData = (word >> (halfIndex * 16u)) & 0xFFFFu;
        
        return voxelData != 0u;
    }
}

// Get voxel color from hierarchical storage
fn getVoxelColor(voxelPos: vec3<i32>) -> vec4<f32> {
    // Check bounds
    if (voxelPos.x < 0 || voxelPos.x >= WORLD_SIZE ||
        voxelPos.y < 0 || voxelPos.y >= WORLD_SIZE ||
        voxelPos.z < 0 || voxelPos.z >= WORLD_SIZE) {
        return vec4<f32>(1.0, 0.0, 1.0, 1.0); // Magenta for out of bounds
    }

    // Get top-level grid entry using the new texture-based method
    let topEntry = getTopLevelEntry(voxelPos);

    if ((topEntry & IS_BRICK_FLAG) == 0u) {
        // It's a solid color
        if (topEntry == 0u) {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0); // Transparent for empty
        }
        
        // Unpack RGB color from bits 0-23
        let r = f32((topEntry >> 16u) & 0xFFu) / 255.0;
        let g = f32((topEntry >> 8u) & 0xFFu) / 255.0;
        let b = f32(topEntry & 0xFFu) / 255.0;
        return vec4<f32>(r, g, b, 1.0);
    } else {
        // It's a brick - check individual voxel
        let brickIndex = topEntry & BRICK_INDEX_MASK;
        
        // Bounds check for brick index
        if (brickIndex >= MAX_BRICK_POOL_SIZE) {
            return vec4<f32>(1.0, 0.0, 1.0, 1.0); // Magenta for invalid brick
        }
        
        let localX = voxelPos.x % BRICK_SIZE;
        let localY = voxelPos.y % BRICK_SIZE;
        let localZ = voxelPos.z % BRICK_SIZE;
        
        // Each brick has 512 uint16 voxels, packed 2 per u32 (= 256 u32 per brick)
        let voxelIndex = brickIndex * 512u + u32(localX + localY * BRICK_SIZE + localZ * BRICK_SIZE * BRICK_SIZE);
        let wordIndex = voxelIndex / 2u;   // 2 uint16s per u32
        let halfIndex = voxelIndex % 2u;   // which uint16 within the u32 (0=low, 1=high)
        
        let word = brickPool[wordIndex];
        let voxelData = (word >> (halfIndex * 16u)) & 0xFFFFu;
        
        if (voxelData == 0u) {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0); // Transparent for empty
        }
        
        // Lower 8 bits = palette index (upper 8 bits reserved for future material data)
        let paletteIndex = voxelData & 0xFFu;
        
        // Look up color from palette
        let packedColor = colorPalette[paletteIndex];
        let r = f32((packedColor >> 16u) & 0xFFu) / 255.0;
        let g = f32((packedColor >> 8u) & 0xFFu) / 255.0;
        let b = f32(packedColor & 0xFFu) / 255.0;
        let a = f32((packedColor >> 24u) & 0xFFu) / 255.0;
        
        return vec4<f32>(r, g, b, a);
    }
}

// Multi-level heatmap (Viridis/Inferno blend)
fn viridis_inferno(t: f32) -> vec3<f32> {
    let x = clamp(t, 0.0, 1.0);
    // Simple but vibrant inferno-like color ramp
    let c1 = vec3<f32>(0.0, 0.0, 0.04);       // Black/Purple
    let c2 = vec3<f32>(0.5, 0.0, 0.5);        // Purple/Magenta
    let c3 = vec3<f32>(1.0, 0.5, 0.0);        // Orange
    let c4 = vec3<f32>(1.0, 1.0, 0.5);        // Yellow
    
    if (x < 0.33) {
        return mix(c1, c2, x / 0.33);
    } else if (x < 0.66) {
        return mix(c2, c3, (x - 0.33) / 0.33);
    } else {
        return mix(c3, c4, (x - 0.66) / 0.34);
    }
}

// Visualize ray steps with clear contrast - Heatmap version
fn visualizeRaySteps(steps: i32, maxSteps : i32) -> vec4<f32> {
    if (steps <= 0) {
        return vec4<f32>(0.0, 0.0, 0.0, 1.0);
    }

    let iters = f32(steps) / f32(maxSteps);
    let color = viridis_inferno(iters);
    return vec4<f32>(color, 1.0);
}

// Calculate shadow by casting a ray towards the light
fn calculateShadow(hitPos: vec3<f32>, normal : vec3<f32>, lightDir : vec3<f32>, gridMin : vec3<i32>, gridMax : vec3<i32>, cameraPos: vec3<f32>, cameraPosInt: vec3<i32>, cameraPosOffset: vec3<f32>) -> f32 {
    // Offset the ray start position slightly along the normal to avoid self-intersection
    // Using 0.1 as a precision-safe offset for 1.0 unit voxels
    let shadowRayOriginWorld = hitPos + normal * 0.1;
    let shadowRayOriginRelative = shadowRayOriginWorld - cameraPos;

    // Cast shadow ray towards light using the main DDA function
    let shadowHit = hierarchicalVoxelDDA(shadowRayOriginRelative, lightDir, gridMin, gridMax, cameraPos, cameraPosInt, cameraPosOffset);

    // If we hit something solid, we're in shadow
    if (shadowHit.t > 0.0 && isVoxelSolid(shadowHit.voxel)) {
        return 0.2; // Shadow intensity
    }

    return 1.0; // No shadow
}

// Calculate reflection color by casting a reflection ray
fn calculateReflection(hitPos: vec3<f32>, rayDir : vec3<f32>, normal : vec3<f32>, gridMin : vec3<i32>, gridMax : vec3<i32>, cameraPos: vec3<f32>, cameraPosInt: vec3<i32>, cameraPosOffset: vec3<f32>) -> vec4<f32> {
    // Calculate reflection direction
    let reflectDir = reflect(rayDir, normal);

    // Offset the ray start position slightly along the normal to avoid self-intersection
    let reflectRayOriginWorld = hitPos + normal * 0.01;
    let reflectRayOriginRelative = reflectRayOriginWorld - cameraPos;

    // Cast reflection ray
    let reflectHit = hierarchicalVoxelDDA(reflectRayOriginRelative, reflectDir, gridMin, gridMax, cameraPos, cameraPosInt, cameraPosOffset);

    if (reflectHit.t > 0.0) {
        let reflectedVoxelColor = getVoxelColor(reflectHit.voxel);
        if (reflectedVoxelColor.a > 0.0) {
            // Apply simple lighting to reflected surface
            let lightDir = normalize(vec3<f32>(1.0, 1.0, 1.0));
            let reflectedDiffuse = max(0.3, dot(reflectHit.normal, lightDir));
            return vec4<f32>(reflectedVoxelColor.rgb * reflectedDiffuse, reflectedVoxelColor.a);
        }
    }

    // If no reflection hit, return sky color
    return vec4<f32>(0.5, 0.7, 1.0, 0.1); // Low alpha for subtle reflection
}

// Cheap voxel face AO: sample neighbors around the hit face.
fn calculateVoxelAO(voxelPos: vec3<i32>, normal: vec3<f32>) -> f32 {
    let n = vec3<i32>(i32(round(normal.x)), i32(round(normal.y)), i32(round(normal.z)));

    var tangentA = vec3<i32>(1, 0, 0);
    var tangentB = vec3<i32>(0, 1, 0);
    if (abs(n.x) == 1) {
        tangentA = vec3<i32>(0, 1, 0);
        tangentB = vec3<i32>(0, 0, 1);
    } else if (abs(n.y) == 1) {
        tangentA = vec3<i32>(1, 0, 0);
        tangentB = vec3<i32>(0, 0, 1);
    } else {
        tangentA = vec3<i32>(1, 0, 0);
        tangentB = vec3<i32>(0, 1, 0);
    }

    // Sample one voxel outside the hit face to bias AO toward exposed areas.
    let faceCell = voxelPos + n;

    let sideOcc =
        f32(isVoxelSolid(faceCell + tangentA)) +
        f32(isVoxelSolid(faceCell - tangentA)) +
        f32(isVoxelSolid(faceCell + tangentB)) +
        f32(isVoxelSolid(faceCell - tangentB));

    let cornerOcc =
        f32(isVoxelSolid(faceCell + tangentA + tangentB)) +
        f32(isVoxelSolid(faceCell + tangentA - tangentB)) +
        f32(isVoxelSolid(faceCell - tangentA + tangentB)) +
        f32(isVoxelSolid(faceCell - tangentA - tangentB));

    // Side neighbors contribute more than corners.
    let occlusion = sideOcc * 0.12 + cornerOcc * 0.07;
    return clamp(1.0 - occlusion, 0.35, 1.0);
}

//@compute @workgroup_size(8, 8, 1)
//fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
//    let dims = textureDimensions(outTex);
//    if (gid.x >= dims.x || gid.y >= dims.y) { return; }
//
//    // TEMPORARY: Render owl texture instead of voxels
//    let resolution = res.xy;
//    let pixelCenter = vec2<f32>(f32(gid.x) + 0.5, f32(gid.y) + 0.5);
//    let uv = pixelCenter / resolution;
//    
//    // Get texture dimensions and convert UV to pixel coordinates
//    let texDims = textureDimensions(owlTexture);
//    let texCoords = vec2<i32>(
//        i32(uv.x * f32(texDims.x)),
//        i32(uv.y * f32(texDims.y))
//    );
//    
//    // Clamp coordinates to texture bounds
//    let clampedCoords = vec2<i32>(
//        clamp(texCoords.x, 0, i32(texDims.x) - 1),
//        clamp(texCoords.y, 0, i32(texDims.y) - 1)
//    );
//    
//    // Load the owl texture pixel (use textureLoad instead of textureSample for compute)
//    let owlColor = textureLoad(owlTexture, clampedCoords, 0);
//    
//    // Store the texture color directly
//    textureStore(outTex, vec2<i32>(i32(gid.x), i32(gid.y)), owlColor);
//}

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
    let dims = textureDimensions(outTex);
    if (gid.x >= dims.x || gid.y >= dims.y) { return; }

    let resolution = res.xy;
    // Use pixel center for ray generation with improved precision
    let pixelCenter = vec2<f32>(f32(gid.x) + 0.5, f32(gid.y) + 0.5);
    let uv = (pixelCenter / resolution) * 2.0 - vec2<f32>(1.0, 1.0);
    let ndc = vec4<f32>(uv.x, -uv.y, 1.0, 1.0);

    // PRECISION FIX: Use camera-relative coordinates
    let cameraPos = position;
    let cameraPosInt = vec3<i32>(i32(floor(cameraPos.x)), i32(floor(cameraPos.y)), i32(floor(cameraPos.z)));
    let cameraPosOffset = cameraPos - vec3<f32>(cameraPosInt);

    let rayOrigin = vec3<f32>(0.0, 0.0, 0.0);
    let viewPos = invViewProj * ndc;
    let rayDir = normalize(viewPos.xyz / viewPos.w);

    let gridMin = vec3<i32>(0, 0, 0);
    let gridMax = vec3<i32>(WORLD_SIZE - 1, WORLD_SIZE - 1, WORLD_SIZE - 1);

    let hit = hierarchicalVoxelDDA(rayOrigin, rayDir, gridMin, gridMax, cameraPos, cameraPosInt, cameraPosOffset);

    var color : vec4<f32>;

    // If we hit a voxel, return its color with shadows and reflections
    if (hit.t > 0.0) {
        let voxelColor = getVoxelColor(hit.voxel);
        if (voxelColor.a > 0.0) {
            // Calculate hit position in world space
            let hitPos = rayOrigin + hit.t * rayDir;

            // Apply lighting based on normal
            let lightDir = normalize(vec3<f32>(1.0, 1.0, 1.0));
            let nDotL = dot(hit.normal, lightDir);
            
            var shadowFactor: f32 = 1.0;
            if (nDotL <= 0.0) {
                // Face is pointing away from the light, it's automatically in shadow
                shadowFactor = 0.2;
            } else {
                // Regular per-pixel shadow tracing for faces pointing towards the light
                shadowFactor = calculateShadow(hitPos + cameraPos, hit.normal, lightDir, gridMin, gridMax, cameraPos, cameraPosInt, cameraPosOffset);
            }

            // Ambient occlusion helps reveal voxel edges and corners.
            let ao = calculateVoxelAO(hit.voxel, hit.normal);
            let ambient = 0.26 * ao;
            let direct = max(0.0, nDotL) * shadowFactor;
            let light = ambient + direct * 0.82;
            let shadedColor = voxelColor.rgb * light;
            let finalColor = shadedColor;

            color = vec4<f32>(finalColor, 1.0);
        }
        else {
            // Show step visualization if ray entered the voxel grid but didn't hit anything
            if (hit.steps >= 0 && res.z > 0.5) {
                let maxSteps = 512;
                color = visualizeRaySteps(hit.steps, maxSteps);
            }
            else {
                // Ray missed the voxel grid - show blue sky
                color = vec4<f32>(0.5, 0.7, 1.0, 1.0);
            }
        }
    }
    else {
        // Show step visualization if ray entered the voxel grid but didn't hit anything
        if (hit.steps >= 0 && res.z > 0.5) {
            let maxSteps = 512;
            color = visualizeRaySteps(hit.steps, maxSteps);
        }
        else {
            // Ray missed the voxel grid - show blue sky
            color = vec4<f32>(0.5, 0.7, 1.0, 1.0);
        }
    }

    textureStore(outTex, vec2<i32>(i32(gid.x), i32(gid.y)), color);
}