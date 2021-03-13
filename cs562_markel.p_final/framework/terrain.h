/*

	TERRAIN

	Geometry Clipmaps

*/

#pragma once

#include "mesh.h"
#include "camera.h"

#define TERRAIN_MESH_COUNT 4	// center, side, corner, ring
#define TERRAIN_CENTER_GRID	0	// remember the order
#define TERRAIN_SIDE_GRID	1
#define TERRAIN_CORNER_GRID	2
#define TERRAIN_RING_MESH	3

void create_grid_with_margins_mesh(MeshBuffers* mb, MeshData* md, tri_idx segments, f32 margin_proportion, std::array<bool, 4> directions	/* +X, -X, +Y, -Y */);
void create_margins_ring_mesh(MeshBuffers* mb, MeshData* md, tri_idx segments, f32 margin_proportion);
void create_terrain_meshes(MeshBuffers (&buffers)[TERRAIN_MESH_COUNT], u32 segments, f32 margin_proportion);

void draw_terrain_LOD(Shader sh, const MeshBuffers(&buffers)[TERRAIN_MESH_COUNT], Camera cam, u8 levels, f32 snap_precision, bool frustrum_culling);

