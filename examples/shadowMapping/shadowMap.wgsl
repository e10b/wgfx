struct Scene {
	lightViewProjMatrix: mat4x4f,
	cameraViewProjMatrix: mat4x4f,
	modelMatrix: mat4x4f,
	lightPos: vec4f,
};

@group(0) @binding(0) var<uniform> scene: Scene;

@vertex
fn vs_main(@location(0) position: vec3f) -> @builtin(position) vec4f {
	return scene.lightViewProjMatrix * scene.modelMatrix * vec4f(position, 1.0);
}

@fragment
fn fs_main() {
}
