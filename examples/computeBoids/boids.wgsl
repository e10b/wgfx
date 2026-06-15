struct Particle {
	pos: vec2f,
	vel: vec2f,
};

struct SimParams {
	deltaT: f32,
	rule1Distance: f32,
	rule2Distance: f32,
	rule3Distance: f32,
	rule1Scale: f32,
	rule2Scale: f32,
	rule3Scale: f32,
};

struct Particles {
	particles: array<Particle>,
};

@group(0) @binding(0) var<uniform> params: SimParams;
@group(0) @binding(1) var<storage, read> particlesA: Particles;
@group(0) @binding(2) var<storage, read_write> particlesB: Particles;

@compute @workgroup_size(64)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID: vec3u) {
	let index = GlobalInvocationID.x;
	if (index >= arrayLength(&particlesA.particles)) {
		return;
	}
	var vPos = particlesA.particles[index].pos;
	var vVel = particlesA.particles[index].vel;
	var cMass = vec2f(0.0);
	var cVel = vec2f(0.0);
	var colVel = vec2f(0.0);
	var cMassCount = 0u;
	var cVelCount = 0u;
	for (var i = 0u; i < arrayLength(&particlesA.particles); i++) {
		if (i == index) {
			continue;
		}
		let pos = particlesA.particles[i].pos;
		let vel = particlesA.particles[i].vel;
		if (distance(pos, vPos) < params.rule1Distance) {
			cMass += pos;
			cMassCount++;
		}
		if (distance(pos, vPos) < params.rule2Distance) {
			colVel -= pos - vPos;
		}
		if (distance(pos, vPos) < params.rule3Distance) {
			cVel += vel;
			cVelCount++;
		}
	}
	if (cMassCount > 0) {
		cMass = (cMass / vec2f(f32(cMassCount))) - vPos;
	}
	if (cVelCount > 0) {
		cVel /= f32(cVelCount);
	}
	vVel += (cMass * params.rule1Scale) + (colVel * params.rule2Scale) + (cVel * params.rule3Scale);
	vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);
	vPos += vVel * params.deltaT;
	if (vPos.x < -1.0) { vPos.x = 1.0; }
	if (vPos.x > 1.0) { vPos.x = -1.0; }
	if (vPos.y < -1.0) { vPos.y = 1.0; }
	if (vPos.y > 1.0) { vPos.y = -1.0; }
	particlesB.particles[index].pos = vPos;
	particlesB.particles[index].vel = vVel;
}

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(4) color: vec4f,
};

@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32, @builtin(instance_index) instanceIndex: u32) -> VertexOutput {
	let particle = particlesA.particles[instanceIndex];
	var tri = array<vec2f, 3>(vec2f(-0.01, -0.02), vec2f(0.01, -0.02), vec2f(0.0, 0.02));
	let a_pos = tri[vertexIndex];
	let angle = -atan2(particle.vel.x, particle.vel.y);
	let pos = vec2f((a_pos.x * cos(angle)) - (a_pos.y * sin(angle)), (a_pos.x * sin(angle)) + (a_pos.y * cos(angle)));
	var output: VertexOutput;
	output.position = vec4f(pos + particle.pos, 0.0, 1.0);
	output.color = vec4f(1.0 - sin(angle + 1.0) - particle.vel.y, pos.x * 100.0 - particle.vel.y + 0.1, particle.vel.x + cos(angle + 0.5), 1.0);
	return output;
}

@fragment
fn fs_main(@location(4) color: vec4f) -> @location(0) vec4f {
	return color;
}
