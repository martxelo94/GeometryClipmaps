/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: mesh.h
Purpose: define MeshData (CPU) and MeshBuffers (GPU)
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include "graphics.h"
#include <array>
#include "assimp\mesh.h"
#include "assimp\scene.h"


#define VBO_SIZE 6		//positions, normals, uvs, colors, tangent, bitangent
#define DEFAULT_SEGMENTS 16

/*
	Vertex Attributes:	position (0), normal (1), uvs/texture_coords (2), color (3), tangent (4), bitangent (5)... 
	Vertex Formats:
		(P)		-> position
		(PC)	-> position, color
		(PN)	-> position, normal
		(PT)	-> position, texture_coords
		(PNT)	-> position, normal, texture
		(PNC)	-> position, normal, color
		(PNTC)	-> position, normal, texture, color
		(PNTtb)	-> position, normal, texture, tangent, bitangent
		(PNTCtb)-> position, normal, texture, color, tangent, bitangent
*/

/*
	Choose triangle index format
#define MESH_IDX_TYPE GL_UNSIGNED_SHORT
*/
#define MESH_IDX_TYPE GL_UNSIGNED_INT
#if MESH_IDX_TYPE == GL_UNSIGNED_INT
using tri_idx = u32;
#elif MESH_IDX_TYPE == GL_UNSIGNED_SHORT
using tri_idx = u16;
#elif MESH_IDX_TYPE == GL_UNSIGNED_BYTE
using tri_idx = u8;
#endif

struct MeshData
{
	//define arrays for each attribute
	std::vector<vec3> positions, normals, tangents, bitangents;
	std::vector<vec2> uvs;
	std::vector<Color> colors;
	std::vector<tri_idx> indices;

	MeshData() = default;
	MeshData(const aiMesh &assimp_mesh);

	inline void clear() {
		positions.clear();
		normals.clear();
		tangents.clear();
		bitangents.clear();
		uvs.clear();
		colors.clear();
		indices.clear();
	}
};

struct MeshBuffers
{
	GLuint vao = 0;
	GLuint vbo[VBO_SIZE] = { 0 };
	GLuint ibo = 0;
	tri_idx index_count = 0;

	MeshBuffers() {}
	MeshBuffers(const aiMesh &assimp_mesh);
	MeshBuffers(MeshBuffers && rhs);
	MeshBuffers& operator=(MeshBuffers &&rhs);
	MeshBuffers(const MeshBuffers &) = delete;
	~MeshBuffers();

	// free GPU memory
	inline void clear() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(VBO_SIZE, vbo);
		glDeleteBuffers(1, &ibo);
		index_count = 0;
	}

	inline void draw(GLuint mode = GL_TRIANGLES) const { glDrawElements(mode, index_count, MESH_IDX_TYPE, 0); }
};

std::vector<tri_idx> faces_to_triangles(const aiFace * assimp_faces, u32 faceNum);

void create_buffers(MeshBuffers* mb,
	vec3 const* positions, vec3 const* normals, vec2 const* uvs, Color const* colors, vec3 const* tangents, vec3 const* bitangents, const tri_idx vertex_count,
	tri_idx const* indices, const tri_idx index_count);
void create_buffers(MeshBuffers* mb, const MeshData *md);

void create_quad_mesh(MeshBuffers* mb, MeshData* md = nullptr);
void create_plane_mesh(MeshBuffers* mb, MeshData* md = nullptr);
void create_grid_mesh(MeshBuffers* mb, MeshData* md = nullptr, tri_idx segments = 1);
void create_cube_mesh(MeshBuffers* mb, MeshData* md = nullptr);
void create_cone_mesh(MeshBuffers* mb, MeshData* md = nullptr, u16 segments = DEFAULT_SEGMENTS);
void create_cylinder_mesh(MeshBuffers* mb, MeshData* md = nullptr, u16 segments = DEFAULT_SEGMENTS);
void create_sphere_mesh( MeshBuffers* mb, MeshData* md = nullptr, u16 slices = DEFAULT_SEGMENTS, u16 rings = DEFAULT_SEGMENTS / 2);
void create_icosahedron_mesh(MeshBuffers* mb, MeshData* md, u8 tesselation_level = 0);
void create_frustrum_mesh(MeshBuffers* mb, MeshData* md, f32 near, f32 far, f32 fov);

bool load_mesh_obj(const char* filepath, MeshBuffers* mb, MeshData* md = nullptr);
bool save_mesh_obj(const char* filepath, const MeshData& md, bool export_tangents = true);

void compute_tangent_basis(vec3 * tangents, vec3 * bitangents,
	const vec3 *positions, const vec3 *normals, const vec2 *uvs, const tri_idx vertex_count,
	const tri_idx *indices, const tri_idx index_count);
void compute_tangent_basis(MeshData &md);
void spherical_uvs(vec2 * uvs, vec3 const* positions, tri_idx vertex_count);
MeshData tesselate(const MeshData& md, u8 level, bool normalized_output = false);


using TriAdj = std::array<int, 3>;
//input indices are modified so adjacency list output matches indices
std::vector<TriAdj> adjacent_triangles(tri_idx * indices, const tri_idx index_count);

//draw a 2D voronoi graph
void draw_voronoi_graph(vec3 const* positions,
	tri_idx const *indices,
	TriAdj const* triangle_adjacency, tri_idx triangle_count, Color color);

#include <set>
using EdgeGraph = std::vector<std::set<tri_idx>>;
//create an EdgeGraph from a list of triangles
EdgeGraph edge_graph(tri_idx const *indices, tri_idx tri_count, tri_idx vertex_count);
