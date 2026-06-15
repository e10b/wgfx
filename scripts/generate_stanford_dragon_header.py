#!/usr/bin/env python3
import ast
import math
import re
from pathlib import Path


INPUT = Path("/private/tmp/webgpu-samples/meshes/stanfordDragonData.ts")
OUTPUT = Path("examples/common/stanfordDragonMesh.h")


def sub(a, b):
	return [a[0] - b[0], a[1] - b[1], a[2] - b[2]]


def dot(a, b):
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def cross(a, b):
	return [
		a[1] * b[2] - a[2] * b[1],
		a[2] * b[0] - a[0] * b[2],
		a[0] * b[1] - a[1] * b[0],
	]


def normalize(v):
	length = math.sqrt(dot(v, v))
	return [v[0] / length, v[1] / length, v[2] / length] if length else [0, 0, 0]


def generate_normals(max_angle, positions, triangles):
	face_normals = [
		normalize(cross(sub(positions[n2], positions[n1]), sub(positions[n3], positions[n1])))
		for n1, n2, n3 in triangles
	]

	shared_map = {}
	shared_indices = []
	for position in positions:
		key = repr(position)
		if key not in shared_map:
			shared_map[key] = len(shared_map)
		shared_indices.append(shared_map[key])

	vert_faces = [[] for _ in range(len(shared_map))]
	for face_index, tri in enumerate(triangles):
		for index in tri:
			vert_faces[shared_indices[index]].append(face_index)

	max_angle_cos = math.cos(max_angle)
	new_positions = []
	new_normals = []
	new_triangles = []
	new_vert_map = {}

	def get_new_index(position, normal):
		key = repr((position, normal))
		if key not in new_vert_map:
			new_vert_map[key] = len(new_positions)
			new_positions.append(position)
			new_normals.append(normal)
		return new_vert_map[key]

	for face_index, tri in enumerate(triangles):
		new_tri = []
		for index in tri:
			normal = [0, 0, 0]
			for other_face in vert_faces[shared_indices[index]]:
				if dot(face_normals[face_index], face_normals[other_face]) > max_angle_cos:
					normal = [
						normal[0] + face_normals[other_face][0],
						normal[1] + face_normals[other_face][1],
						normal[2] + face_normals[other_face][2],
					]
			new_tri.append(get_new_index(positions[index], normalize(normal)))
		new_triangles.append(new_tri)

	return new_positions, new_normals, new_triangles


def compute_projected_plane_uvs(positions):
	uvs = [[p[0], p[1]] for p in positions]
	min_u = min(uv[0] for uv in uvs)
	min_v = min(uv[1] for uv in uvs)
	max_u = max(uv[0] for uv in uvs)
	max_v = max(uv[1] for uv in uvs)
	return [[(uv[0] - min_u) / (max_u - min_u), (uv[1] - min_v) / (max_v - min_v)] for uv in uvs]


def wrap(values, per_line, formatter):
	lines = []
	for index, value in enumerate(values):
		if index % per_line == 0:
			lines.append("\t\t")
		lines[-1] += formatter(value) + ", "
	return "\n".join(lines)


def float_literal(value):
	value = float(value)
	if value.is_integer():
		return f"{value:.1f}f"
	return f"{value:.9g}f"


def main():
	raw = INPUT.read_text()
	raw = re.sub(r"//.*\n", "", raw)
	raw = re.sub(r"^\s*export\s+default\s*", "", raw)
	raw = re.sub(r";\s*$", "", raw)
	raw = re.sub(r"([A-Za-z_][A-Za-z0-9_]*)\s*:", r"'\1':", raw)
	raw = re.sub(r",\s*([}\]])", r"\1", raw)
	data = ast.literal_eval(raw)

	positions, normals, triangles = generate_normals(math.pi, data["positions"], data["cells"])
	uvs = compute_projected_plane_uvs(positions)

	triangles.extend([
		[len(positions), len(positions) + 2, len(positions) + 1],
		[len(positions), len(positions) + 1, len(positions) + 3],
	])
	positions.extend([[-100, 20, -100], [100, 20, 100], [-100, 20, 100], [100, 20, -100]])
	normals.extend([[0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0]])
	uvs.extend([[0, 0], [1, 1], [0, 1], [1, 0]])

	shadow_vertices = []
	deferred_vertices = []
	for position, normal, uv in zip(positions, normals, uvs):
		shadow_vertices.extend(position + normal)
		deferred_vertices.extend(position + normal + uv)
	indices = [index for tri in triangles for index in tri]
	if max(indices) > 65535:
		raise RuntimeError("dragon indices no longer fit uint16_t")

	OUTPUT.parent.mkdir(parents=True, exist_ok=True)
	OUTPUT.write_text(f"""#pragma once

#include <cstdint>
#include <vector>

namespace sample_mesh
{{
inline std::vector<float> stanfordDragonShadowVertices()
{{
\treturn {{
{wrap(shadow_vertices, 6, float_literal)}
\t}};
}}

inline std::vector<float> stanfordDragonDeferredVertices()
{{
\treturn {{
{wrap(deferred_vertices, 8, float_literal)}
\t}};
}}

inline std::vector<uint16_t> stanfordDragonIndices()
{{
\treturn {{
{wrap(indices, 18, str)}
\t}};
}}
}}
""")
	print(f"wrote {OUTPUT}: {len(positions)} vertices, {len(indices)} indices")


if __name__ == "__main__":
	main()
