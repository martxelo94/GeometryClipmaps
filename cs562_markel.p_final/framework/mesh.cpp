/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: mesh.cpp
Purpose: implement bunch of primitive's creations
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#include "mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/hash.hpp"
#include <unordered_map>	// edges -> point triplets for triangle adjacency
#include <map>
#include <set>	// order sorted triangles
#include <fstream>	//ifstream on load_mesh_obj
#include <iomanip>

MeshData::MeshData(const aiMesh &assimp_mesh)
{
	positions.assign(reinterpret_cast<vec3*>(assimp_mesh.mVertices), reinterpret_cast<vec3*>(assimp_mesh.mVertices + assimp_mesh.mNumVertices));
	normals.assign(reinterpret_cast<vec3*>(assimp_mesh.mNormals), reinterpret_cast<vec3*>(assimp_mesh.mNormals + assimp_mesh.mNumVertices));
	tangents.assign(reinterpret_cast<vec3*>(assimp_mesh.mTangents), reinterpret_cast<vec3*>(assimp_mesh.mTangents+ assimp_mesh.mNumVertices));
	bitangents.assign(reinterpret_cast<vec3*>(assimp_mesh.mBitangents), reinterpret_cast<vec3*>(assimp_mesh.mBitangents + assimp_mesh.mNumVertices));
	
	if (assimp_mesh.mTextureCoords[0] != nullptr) {
		uvs.reserve(assimp_mesh.mNumVertices);
		for (u32 i = 0; i < assimp_mesh.mNumVertices; i++) {
			aiVector3D texCoord = assimp_mesh.mTextureCoords[0][i];
			uvs.push_back(vec2{ texCoord.x, texCoord.y });
		}
	}
	if (assimp_mesh.mColors[0] != nullptr) {
		colors.reserve(assimp_mesh.mNumVertices);
		for (u32 i = 0; i < assimp_mesh.mNumVertices; i++) {
			aiColor4D c = assimp_mesh.mColors[0][i];
			colors.push_back(Color{ vec4{c.r, c.g, c.b, c.a} });
		}
	}
	indices = faces_to_triangles(assimp_mesh.mFaces, assimp_mesh.mNumFaces);

}

MeshBuffers::MeshBuffers(const aiMesh &assimp_mesh)
{
	std::vector<tri_idx> indices = faces_to_triangles(assimp_mesh.mFaces, assimp_mesh.mNumFaces);
	std::vector<vec2> uvs;
	uvs.reserve(assimp_mesh.mNumVertices);
	for (u32 i = 0; i < assimp_mesh.mNumVertices; i++) {
		aiVector3D texCoord = assimp_mesh.mTextureCoords[0][i];
		uvs.push_back(vec2{ texCoord.x, texCoord.y });
	}
	std::vector<Color> colors;
	colors.reserve(assimp_mesh.mNumVertices);
	if (assimp_mesh.HasVertexColors(0))
		for (u32 i = 0; i < assimp_mesh.mNumVertices; i++) {
			aiColor4D c = assimp_mesh.mColors[0][i];
			colors.push_back(Color{ vec4{ c.r, c.g, c.b, c.a } });
		}
	else
		colors = std::vector<Color>(assimp_mesh.mNumVertices, Color{ 255, 255, 255,255 });

	// load to GPU!
	create_buffers(this, reinterpret_cast<vec3 const*>(assimp_mesh.mVertices), reinterpret_cast<vec3 const*>(assimp_mesh.mNormals),
		uvs.data(), colors.data(), 
		reinterpret_cast<vec3 const*>(assimp_mesh.mTangents), reinterpret_cast<vec3 const*>(assimp_mesh.mBitangents),
		assimp_mesh.mNumVertices,
		indices.data(), indices.size());
}



std::vector<tri_idx> faces_to_triangles(const aiFace * assimp_faces, u32 faceNum)
{
	std::vector<tri_idx> triangle_indices;
	triangle_indices.reserve(faceNum * 3);
	for (u32 i = 0; i < faceNum; i++) {
		const aiFace& face = assimp_faces[i];
		assert(face.mNumIndices > 2);
		tri_idx first_idx = face.mIndices[0];
		for (u32 idx = 1; idx < face.mNumIndices - 1; idx++) {
			triangle_indices.push_back(first_idx);
			triangle_indices.push_back(face.mIndices[idx]);
			triangle_indices.push_back(face.mIndices[idx + 1]);
		}
	}
	return triangle_indices;
}


MeshBuffers::MeshBuffers(MeshBuffers && rhs) 
{
	assert(vao == 0);
	//pass data
	vao = rhs.vao;
	std::memcpy(vbo, rhs.vbo, sizeof(GLuint) * VBO_SIZE);
	ibo = rhs.ibo;
	index_count = rhs.index_count;
	//erase data
	rhs.vao = rhs.ibo = rhs.index_count = 0;
	std::memset(rhs.vbo, 0, sizeof(GLuint) * VBO_SIZE);
}

MeshBuffers& MeshBuffers::operator=(MeshBuffers &&rhs)
{
	//pass data
	glDeleteVertexArrays(1, &vao);
	vao = rhs.vao;
	glDeleteBuffers(VBO_SIZE, vbo);
	std::memcpy(vbo, rhs.vbo, sizeof(GLuint) * VBO_SIZE);
	glDeleteBuffers(1, &ibo);
	ibo = rhs.ibo;
	index_count = rhs.index_count;
	//erase data
	rhs.vao = rhs.ibo = rhs.index_count = 0;
	std::memset(rhs.vbo, 0, sizeof(GLuint) * VBO_SIZE);
	return *this;
}

MeshBuffers::~MeshBuffers() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(VBO_SIZE, vbo);
	glDeleteBuffers(1, &ibo);
}

void create_buffers(MeshBuffers* mb,
	vec3 const* positions, vec3 const* normals, vec2 const* uvs, Color const* colors, vec3 const* tangents, vec3 const* bitangents, const tri_idx vertex_count,
	tri_idx const* indices, const tri_idx index_count)
{
	assert(mb);
	mb->clear();

	glGenVertexArrays(1, &mb->vao);
	glBindVertexArray(mb->vao);
	//generate buffers
	glGenBuffers(VBO_SIZE, &mb->vbo[0]);
	//load positions buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertex_count, positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//load normals buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertex_count, normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
	//load uvs buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertex_count, uvs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//load color buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Color) * vertex_count, colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	//load tangents
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertex_count, tangents, GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//load bitangent
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertex_count, bitangents, GL_STATIC_DRAW);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//load index buffer
	glGenBuffers(1, &mb->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mb->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tri_idx) * index_count, indices, GL_STATIC_DRAW);
	mb->index_count = index_count;
	//Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	pop_gl_errors(__FUNCTION__);
}

void create_buffers(MeshBuffers* mb, const MeshData *md)
{
	glGenVertexArrays(1, &mb->vao);
	glBindVertexArray(mb->vao);
	//generate buffers
	glGenBuffers(VBO_SIZE, &mb->vbo[0]);
	//load positions buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * md->positions.size(), md->positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//load normals buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * md->normals.size(), md->normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
	//load uvs buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * md->uvs.size(), md->uvs.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//load color buffer
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Color) * md->colors.size(), md->colors.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	//load tangents
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * md->tangents.size(), md->tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//load bitangent
	glBindBuffer(GL_ARRAY_BUFFER, mb->vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * md->bitangents.size(), md->bitangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//load index buffer
	glGenBuffers(1, &mb->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mb->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tri_idx) * md->indices.size(), md->indices.data(), GL_STATIC_DRAW);
	mb->index_count = md->indices.size();
	
	//Unbind buffers --- NO!!! (allow binding more buffers after this function call...)
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	pop_gl_errors(__FUNCTION__);
}

void create_quad_mesh(MeshBuffers* mb, MeshData* md)
{
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = 4;
	// defined ccw plane on XY axis
	vec3 positions[vertexCount] = {
		{.5f, .5f, 0.f},		// 0
		{-.5f, .5f, 0.f},		// 1
		{-.5f, -.5f, 0.f},	// 2
		{.5f, -.5f, 0.f}		// 3
	};
	vec3 normals[vertexCount] = {
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f}
	};
	vec2 uvs[vertexCount] = {
		{1.f, 1.f},
		{0.f, 1.f},
		{0.f, 0.f},
		{1.f, 0.f}
	};
	Color colors[vertexCount] = { DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR };
	tri_idx indices[6] = {
		0, 1, 2,
		0, 2, 3
	};
	// tangent & bitangent computation
	vec3 tangents[vertexCount] = { vec3{} };
	vec3 bitangents[vertexCount] = { vec3{} };

	compute_tangent_basis(tangents, bitangents, positions, normals, uvs, vertexCount, indices, 6);

	if(mb)
		create_buffers(mb, positions, normals, uvs, colors, tangents, bitangents, vertexCount, indices, sizeof(indices) / sizeof(tri_idx));
	if (md) {
		md->positions.assign(positions, positions + vertexCount);
		md->normals.assign(normals, normals + vertexCount);
		md->uvs.assign(uvs, uvs + vertexCount);
		md->colors.assign(colors, colors + vertexCount);
		md->tangents.assign(tangents, tangents + vertexCount);
		md->bitangents.assign(bitangents, bitangents + vertexCount);
		md->indices.assign(indices, indices + 6);
	}
}
//like the quad, but Y axis aligned
void create_plane_mesh(MeshBuffers* mb, MeshData* md)
{
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = 4;
	// defined ccw plane on XY axis
	vec3 positions[vertexCount] = {
		{.5f, 0.f, .5f,},		// 0
		{-.5f,0.f, .5f,},		// 1
		{-.5f,0.f,-.5f,},	// 2
		{.5f, 0.f,-.5f,}		// 3
	};
	vec3 normals[vertexCount] = {
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f}
	};
	vec2 uvs[vertexCount] = {
		{1.f, 1.f},
		{0.f, 1.f},
		{0.f, 0.f},
		{1.f, 0.f}
	};
	Color colors[vertexCount] = { DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR };

	tri_idx indices[6] = {
		0, 1, 2,
		0, 2, 3
	};
	// tangent & bitangent computation
	vec3 tangents[vertexCount] = { vec3{} };
	vec3 bitangents[vertexCount] = { vec3{} };

	compute_tangent_basis(tangents, bitangents, positions, normals, uvs, vertexCount, indices, 6);

	if (mb)
		create_buffers(mb, positions, normals, uvs, colors, tangents, bitangents, vertexCount, indices, sizeof(indices) / sizeof(tri_idx));
	if (md) {
		md->positions.assign(positions, positions + vertexCount);
		md->normals.assign(normals, normals + vertexCount);
		md->uvs.assign(uvs, uvs + vertexCount);
		md->colors.assign(colors, colors + vertexCount);
		md->tangents.assign(tangents, tangents + vertexCount);
		md->bitangents.assign(bitangents, bitangents + vertexCount);
		md->indices.assign(indices, indices + 6);
	}
}

void create_grid_mesh(MeshBuffers* mb, MeshData* md, tri_idx segments)
{
	assert(segments > 0 && "Segments must be greater than 0!");
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = (segments + 1) * (segments + 1);
	// defined ccw plane on XY axis
	std::vector<vec3> positions;
	positions.reserve(vertexCount);
	std::vector<vec2> uvs;
	uvs.reserve(vertexCount);
	std::vector<tri_idx> indices;
	indices.reserve(segments * segments * 6);

	f32 step = 1.f / segments;
	// construct grid
	for (tri_idx y = 0; y < segments + 1; y++) {
		f32 pos_y = y * step;
		for (tri_idx x = 0; x < segments + 1; x++) {
			f32 pos_x = x * step;
			uvs.push_back(vec2{ pos_x, pos_y });
			positions.push_back(vec3{ pos_x - 0.5f, 0.f,pos_y - 0.5f});
		}
	}
	// set triangles
	for (tri_idx y = 0; y < segments; y++) {
		tri_idx row = y * (segments + 1);
		tri_idx next_row = (y + 1) * (segments + 1);
		for (tri_idx x = 0; x < segments; x++) {

			indices.push_back(row + x);
			indices.push_back(next_row + x);
			indices.push_back(row + x + 1);

			indices.push_back(row + x + 1);
			indices.push_back(next_row + x);
			indices.push_back(next_row + x + 1);
		}
	}

	std::vector<vec3> normals(vertexCount, vec3{ 0.f, 1.f, 0.f });
	std::vector<Color> colors(vertexCount, Color{ DEFAULT_COLOR });


	// tangent & bitangent computation
	std::vector<vec3> tangents(vertexCount, vec3{} );
	std::vector<vec3> bitangents(vertexCount, vec3{});

	compute_tangent_basis(tangents.data(), bitangents.data(), positions.data(), normals.data(), uvs.data(), vertexCount, indices.data(), indices.size());

	if (mb)
		create_buffers(mb, positions.data(), normals.data(), uvs.data(), colors.data(), tangents.data(), bitangents.data(), vertexCount, indices.data(), indices.size());
	if (md) {
		md->positions = std::move(positions);
		md->normals = std::move(normals);
		md->uvs = std::move(uvs);
		md->colors = std::move(colors);
		md->tangents = std::move(tangents);
		md->bitangents = std::move(bitangents);
		md->indices = std::move(indices);
	}
}

void create_cube_mesh(MeshBuffers* mb, MeshData* md) {
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = 4 * 6;	// 4 vertices, 6 faces
										// defined ccw plane on XY axis
	vec3 positions[vertexCount] = {
		// front
		{.5f, .5f, .5f},		// 0
		{-.5f, .5f, .5f},		// 1
		{-.5f, -.5f, .5f},	// 2
		{.5f, -.5f, .5f},		// 3
		// back
		{-.5f, .5f, -.5f},	// 4
		{.5f, .5f,  -.5f},	// 5
		{.5f, -.5f, -.5f},	// 6
		{-.5f, -.5f,-.5f},	// 7
		// top
		{.5f, .5f, -.5f},		// 8
		{-.5f, .5f,-.5f},	// 9
		{-.5f, .5f, .5f},		// 10
		{.5f, .5f,  .5f},		// 11
		// bot
		{.5f,  -.5f,  .5f},	// 12
		{-.5f, -.5f,  .5f},	// 13
		{-.5f, -.5f, -.5f},	// 14
		{.5f,  -.5f, -.5f},	// 15
		//left
		{-.5f, .5f,  .5f},		// 16
		{-.5f, .5f, -.5f},	// 17
		{-.5f, -.5f,-.5f},	// 18
		{-.5f, -.5f, .5f},	// 19
		//right
		{.5f, .5f,  -.5f},		// 20
		{.5f, .5f,   .5f},		// 21
		{.5f, -.5f,  .5f},		// 22
		{.5f, -.5f, -.5f}		// 23
	};
	vec3 normals[vertexCount] = {
		//front
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f},
		{0.f, 0.f, 1.f},
		// back
		{0.f, 0.f, -1.f},
		{0.f, 0.f, -1.f},
		{0.f, 0.f, -1.f},
		{0.f, 0.f, -1.f},
		// top
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f},
		{0.f, 1.f, 0.f},
		// bot
		{0.f, -1.f, 0.f},
		{0.f, -1.f, 0.f},
		{0.f, -1.f, 0.f},
		{0.f, -1.f, 0.f},
		//left
		{-1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		//right
		{1.f, 0.f, 0.f},
		{1.f, 0.f, 0.f},
		{1.f, 0.f, 0.f},
		{1.f, 0.f, 0.f}

	};
	vec2 uvs[vertexCount] = {
		//front
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		// back
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		// top
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		// bot
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		//left
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		//right
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f }
	};
	Color colors[vertexCount] = { DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR ,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR ,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR ,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR };
	tri_idx indices[36] = {
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11,
		12, 13, 14,
		12, 14, 15,
		16, 17, 18,
		16, 18, 19,
		20, 21, 22,
		20, 22, 23
	};
	// tangent & bitangent computation
	vec3 tangents[vertexCount] = { vec3{} };
	vec3 bitangents[vertexCount] = { vec3{} };

	compute_tangent_basis(tangents, bitangents, positions, normals, uvs, vertexCount, indices, 36);

	if(mb)
		create_buffers(mb, positions, normals, uvs, colors, tangents, bitangents, vertexCount, indices, sizeof(indices) / sizeof(tri_idx));
	if (md) {
		md->positions.assign(positions, positions + vertexCount);
		md->normals.assign(normals, normals + vertexCount);
		md->uvs.assign(uvs, uvs + vertexCount);
		md->colors.assign(colors, colors + vertexCount);
		md->tangents.assign(tangents, tangents + vertexCount);
		md->bitangents.assign(bitangents, bitangents + vertexCount);
		md->indices.assign(indices, indices + 36);
	}
}
void create_cone_mesh(MeshBuffers* mb, MeshData* md, u16 segments)
{
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = segments * 3 + 1;
	//positions
	std::vector<vec3> positions{ (size_t)vertexCount };
	const f32 angleStep = (2.0f * (f32)M_PI / segments);
	f32 angle = 0.0f;
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle) / 2;
		f32 y = sin(angle) / 2;
		vec3 bot = vec3{ x, -0.5f, y };
		angle += angleStep;
		positions[i] = {0.f, 0.5f, 0.f};	//side
		positions[i + segments] = bot;		//side
		positions[i + segments * 2] = bot;	//disk
	}
	//bot
	positions[vertexCount - 1] = vec3{ 0.0f, -0.5f, 0.0f };

	//normals
	std::vector<vec3> normals{ vertexCount };
	angle = 0.0f;
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle) / 2;
		f32 y = sin(angle) / 2;
		vec3 dir = vec3{ x, -0.5f,  y };
		vec3 tmp = vec3{ 0.f, 0.5f, 0.f } - dir;
		tmp = glm::cross(tmp, dir);
		tmp = glm::normalize(tmp);
		dir = glm::normalize(dir);
		dir = glm::cross(dir, tmp);
		angle += angleStep;
		normals[i] = dir;					//side
		normals[i + segments] = dir;		//side
		normals[i + segments * 2] = { 0.f, -1.f, 0.f };	//disk
	}
	normals[vertexCount - 1] = { 0.f, -1.f, 0.f };
	//uvs
	angle = 0.0f;
	std::vector<vec2> uvs{ (size_t)vertexCount };
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle) / 2 + 0.5f;
		f32 y = sin(angle) / 2 + 0.5f;
		uvs[i] = { angle / (2 * M_PI), 1.f };
		uvs[i + segments] = { angle / (2 * M_PI), 0.f };
		angle += angleStep;
		uvs[i + segments * 2] = { x, y };

	}
	uvs[vertexCount - 2] = uvs[vertexCount - 1] = vec2{ 0.5f, 0.5f };
	//colors (by default)
	std::vector<Color> colors(vertexCount, Color{ DEFAULT_COLOR });
	//indices
	std::vector<tri_idx> indices;
	indices.reserve(segments * 3 * 4);
	for (u16 i = 0; i < segments - 1; ++i)
	{
		//bot
		indices.push_back(i + segments * 2); indices.push_back(i + segments * 2 + 1);  indices.push_back(vertexCount - 1);
		//side quad
		indices.push_back(i); indices.push_back(i + 1); indices.push_back(i + segments);
		indices.push_back(i + segments); indices.push_back(i + 1); indices.push_back(i + segments + 1);
	} //last segments
	//bot
	indices.push_back(segments * 2);  indices.push_back(vertexCount - 1); indices.push_back(segments * 3 - 1);
	//side
	indices.push_back(0); indices.push_back(segments); indices.push_back(segments - 1);
	indices.push_back(segments); indices.push_back(segments * 2 - 1); indices.push_back(segments - 1);

	// tangent & bitangent computation
	std::vector<vec3> tangents = { vertexCount, vec3{} };
	std::vector<vec3> bitangents = { vertexCount, vec3{} };

	compute_tangent_basis(tangents.data(), bitangents.data(), positions.data(), normals.data(), uvs.data(), vertexCount, indices.data(), indices.size());

	// Data completed, load to GPU
	if(mb)
		create_buffers(mb, positions.data(), normals.data(), uvs.data(), colors.data(), tangents.data(), bitangents.data(), positions.size(), indices.data(), indices.size());
	if (md) {
		md->positions = std::move(positions);
		md->normals = std::move(normals);
		md->uvs = std::move(uvs);
		md->colors = std::move(colors);
		md->tangents = std::move(tangents);
		md->bitangents = std::move(bitangents);
		md->indices = std::move(indices);
	}
}
void create_cylinder_mesh(MeshBuffers* mb, MeshData* md, u16 segments)
{
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = segments * 2 * 2 + 2;
	//positions
	std::vector<vec3> positions { (size_t)vertexCount };
	const f32 angleStep = (2.0f * (f32)M_PI / segments);
	f32 angle = 0.0f;
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle) / 2;
		f32 y = sin(angle) / 2;
		vec3 top = vec3{ x, 0.5f,  y };
		vec3 bot = vec3{ x, -0.5f, y };
		angle += angleStep;
		positions[i] = top;					//side
		positions[i + segments] = bot;		//side
		positions[i + segments * 2] = top;	//disk
		positions[i + segments * 3] = bot;	//disk
	}
	//top	
	positions[vertexCount - 2] = vec3{ 0.0f, 0.5f, 0.0f };
	//bot
	positions[vertexCount - 1] = vec3{ 0.0f, -0.5f, 0.0f };

	//normals
	std::vector<vec3> normals{vertexCount};
	angle = 0.0f;
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle);
		f32 y = sin(angle);
		vec3 dir = vec3{ x, 0.f,  y };
		angle += angleStep;
		normals[i] = dir;					//side
		normals[i + segments] = dir;		//side
		normals[i + segments * 2] = { 0.f, 1.f, 0.f };	//disk
		normals[i + segments * 3] = { 0.f, -1.f, 0.f };	//disk
	}
	normals[vertexCount - 2] = { 0.f, 1.f, 0.f };
	normals[vertexCount - 1] = { 0.f, -1.f, 0.f };
	//uvs
	angle = 0.0f;
	std::vector<vec2> uvs { (size_t)vertexCount };
	for (u16 i = 0; i < segments; ++i)
	{
		f32 x = cos(angle) / 2 + 0.5f;
		f32 y = sin(angle) / 2 + 0.5f;
		uvs[i] = { angle / (2 * M_PI), 1.f };
		uvs[i + segments] = { angle / (2 * M_PI), 0.f };
		angle += angleStep;
		uvs[i + segments * 2] = { 1.0f - x, y };
		uvs[i + segments * 3] = { x, y };
		
	}
	uvs[vertexCount - 2] = uvs[vertexCount - 1] = vec2{ 0.5f, 0.5f };
	//colors (by default)
	std::vector<Color> colors(vertexCount, Color{ DEFAULT_COLOR });
	//indices
	std::vector<tri_idx> indices;
	indices.reserve(segments * 3 * 4);
	for (u16 i = 0; i < segments - 1; ++i)
	{
		//top
		indices.push_back(i + segments * 2);  indices.push_back(vertexCount - 2); indices.push_back(i + segments * 2 + 1);
		//bot
		indices.push_back(i + segments * 3); indices.push_back(i + segments * 3 + 1); indices.push_back(vertexCount - 1);
		//side quad
		indices.push_back(i); indices.push_back(i + 1); indices.push_back(i + segments);
		indices.push_back(i + segments); indices.push_back(i + 1); indices.push_back(i + segments + 1);
	}
	//last segments
	//top
	indices.push_back(segments * 2); indices.push_back(segments * 3 - 1); indices.push_back(vertexCount - 2);
	//bot
	indices.push_back(segments * 3);  indices.push_back(vertexCount - 1); indices.push_back(segments * 4 - 1);
	//side
	indices.push_back(0); indices.push_back(segments); indices.push_back(segments - 1);
	indices.push_back(segments); indices.push_back(segments * 2 - 1); indices.push_back(segments - 1);
	
	// tangent & bitangent computation
	std::vector<vec3> tangents = { vertexCount, vec3{} };
	std::vector<vec3> bitangents = { vertexCount, vec3{} };

	compute_tangent_basis(tangents.data(), bitangents.data(), positions.data(), normals.data(), uvs.data(), vertexCount, indices.data(), indices.size());


	// Data completed, load to GPU
	if(mb)
		create_buffers(mb, positions.data(), normals.data(), uvs.data(), colors.data(), tangents.data(), bitangents.data(), vertexCount, indices.data(), indices.size());
	if (md) {
		md->positions = std::move(positions);
		md->normals = std::move(normals);
		md->uvs = std::move(uvs);
		md->colors = std::move(colors);
		md->tangents = std::move(tangents);
		md->bitangents = std::move(bitangents);
		md->indices = std::move(indices);
	}
}

void spherical_uvs(vec2 * uvs, vec3 const* positions, tri_idx vertex_count)
{
	for (tri_idx i = 0; i < vertex_count; i++) {
		const vec3 &p = positions[i];
		f32 angle = atan2(p.z, p.x);
		f32 x = 0.5f + (angle  / f32(M_PI * 2));
		f32 y = 0.5f - (asin(p.y) / f32(M_PI * 2));
		assert(x >= 0 && x <= 1);
		assert(y >= 0 && y <= 1);
		uvs[i] = { x, y };
	}
}
void create_sphere_mesh(MeshBuffers* mb, MeshData* md, u16 slices, u16 rings)
{
#define SPHERE_RADIUS 1.f
	assert(mb || md);	// one of those for sure, why call it otherwise?
	//calculate vtx count and angles
	const tri_idx vertexCount = rings * slices + 2 + rings;	//extra vertices like poles and overlaped ring begin/end
	const f32 ringStep = (f32)M_PI / rings;	//half circle only
	f32 sliceStep = (2.0f * (f32)M_PI / slices);
	f32 currRing = ringStep, currSlice = 0.0f;
	std::vector<vec3> positions;
	positions.reserve(vertexCount);
	for (u16 r = 0; r < rings; ++r)
	{
		//calculate height
		const f32 y = cos(currRing) * SPHERE_RADIUS;
		//calculate radius
		const f32 radius = sin(currRing) * SPHERE_RADIUS;
		for (u16 s = 0; s < slices; ++s)
		{
			//calculate xz
			const f32 x = radius * cos(currSlice);
			const f32 z = radius * sin(currSlice);
			//create the vertex position
			const vec3 pos{ x, y, z };
			//add vertex position
			positions.push_back(pos);
			currSlice += sliceStep;
		}
		//extra vertex to avoid weird texture mapping
		positions.push_back(vec3{ radius, y, 0.0f });

		currSlice = 0.0f;
		currRing += ringStep;
	}
	//add poles
	positions.push_back(vec3{ 0,  SPHERE_RADIUS, 0 });		//top
	positions.push_back(vec3{ 0, -SPHERE_RADIUS, 0 });	//bot

	//normals are pointing out the sphere perpendicularly, so are the positions itself, but must normalize
	std::vector<vec3> normals;
	normals.reserve(positions.size());
	for (const vec3& p : positions)
		normals.push_back(glm::normalize(p));
	//uvs ... how to map this...
#if 0
	currRing = currSlice = 0.0f;
	std::vector<vec2> uvs;
	uvs.reserve(vertexCount);
	currSlice = 2.0f * (f32)M_PI;
	sliceStep = 2.0f * (f32)M_PI / (slices + 1);
	currRing = (f32)M_PI - ringStep;
	for (u16 r = 0; r < rings; ++r)
	{
		for (u16 s = 0; s < slices + 1; ++s)
		{
			//create the vertex position
			vec2 uv{ (f32)currSlice / (2.0f * (f32)M_PI), currRing / (f32)M_PI };
			//add vertex position
			uvs.push_back(uv);
			currSlice -= sliceStep;
		}
		currSlice = 2.0f * (f32)M_PI;
		currRing -= ringStep;
	}
	//poles
	uvs.push_back({ 0.5f, 1.0f });	//top
	uvs.push_back({ 0.5f, 0.0f });	//bot
#else
	std::vector<vec2> uvs(positions.size(), vec2{});
	spherical_uvs(uvs.data(), positions.data(), uvs.size());
#endif
	//colors (by default)
	std::vector<Color> colors(vertexCount, Color{ DEFAULT_COLOR });


	//time of triangulating
	const tri_idx ringTriangleCount = slices * 2 * 3;
	const tri_idx triangleCount = (ringTriangleCount * (rings - 1)) + (slices * 3 * 2);
	std::vector<tri_idx> indices;
	indices.reserve(triangleCount);
	//middle triangles
	for (u16 r = 0; r < rings - 1; ++r)
	{
		for (u16 s = 0; s < slices; ++s)
		{
			const u16 curr = (r * (slices + 1)) + s;
			//quad
			indices.push_back(curr);
			indices.push_back(curr + 1);
			indices.push_back(curr + slices + 1);

			indices.push_back(curr + 1);
			indices.push_back(curr + slices + 1 + 1);
			indices.push_back(curr + slices + 1);
		}
	}
	//poles
	for (u16 s = 0; s < slices; ++s)
	{
		//top
		indices.push_back(s);
		indices.push_back(vertexCount - 2);
		indices.push_back(s + 1);
		//bot
		indices.push_back((rings - 1)* slices + s);
		indices.push_back(vertexCount - 1);
		indices.push_back((rings - 1)* slices + s + 1);
	}

	//check indices are good
	for (size_t i = 0; i < indices.size(); ++i)
		assert((size_t)indices[i] < positions.size());

	// tangent & bitangent computation
	std::vector<vec3> tangents = { vertexCount, vec3{} };
	std::vector<vec3> bitangents = { vertexCount, vec3{} };

	compute_tangent_basis(tangents.data(), bitangents.data(), positions.data(), normals.data(), uvs.data(), vertexCount, indices.data(), indices.size());

	// Data completed, load to GPU
	if(mb)
		create_buffers(mb, positions.data(), normals.data(), uvs.data(), colors.data(), tangents.data(), bitangents.data(), positions.size(), indices.data(), indices.size());
	if (md) {
		md->positions = std::move(positions);
		md->normals = std::move(normals);
		md->uvs = std::move(uvs);
		md->colors = std::move(colors);
		md->tangents = std::move(tangents);
		md->bitangents = std::move(bitangents);
		md->indices = std::move(indices);
	}
}
void create_icosahedron_mesh(MeshBuffers* mb, MeshData* md, u8 tesselation_level)
{
	const f32 e = 1.f + (f32)sqrt(5) / 4;
	const tri_idx vertexCount = 12;

	vec3 positions[vertexCount] = {
		{-1, e, 0}, {1, e, 0}, {-1, -e, 0}, {1, -e, 0},	// XY plane
		{0, -1, e}, {0, 1, e}, {0, -1, -e}, {0, 1, -e},	// YZ plane
		{e, 0, -1}, {e, 0, 1}, {-e, 0, -1}, {-e, 0, 1}	// XZ plane
	};
	//normalize
	for (int i = 0; i < vertexCount; i++)
		positions[i] = glm::normalize(positions[i]);
	vec3 normals[vertexCount] = {};
	std::memcpy(normals, positions, sizeof(vec3) * vertexCount);

	//uvs : calculate spherical coordinates
	vec2 uvs[vertexCount] = { {0.f, 0.f} };
	spherical_uvs(uvs, positions, vertexCount);

	//colors (by default)
	const Color colors[vertexCount] = { 
		DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR,
		DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR,
		DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR
	};
	//indices
	const tri_idx index_count = 20 * 3;
	tri_idx indices[index_count] = {
		0, 10, 11,		// 0
		0, 11, 5,		// 1
		0, 5, 1,		// 2
		0, 1, 7,		// 3
		0, 7, 10,		// 4
		3, 8, 9,		// 5
		3, 6, 8,		// 6
		3, 2, 6,		// 7
		3, 4, 2,		// 8
		3, 9, 4,		// 9
		2, 11, 10,		// 10
		2, 4, 11,		// 11
		4, 5, 11,		// 12
		4, 9, 5,		// 13
		9, 1, 5,		// 14
		9, 8, 1,		// 15
		8, 7, 1,		// 16
		8, 6, 7,		// 17
		7, 6, 10,		// 18
		6, 2, 10		// 19
	};

	// tangent & bitangent computation
	vec3 tangents[vertexCount] = { vec3{} };
	vec3 bitangents[vertexCount] = { vec3{} };

	compute_tangent_basis(tangents, bitangents, positions, normals, uvs, vertexCount, indices, index_count);

	// Data completed, load to GPU
	if (md) {
		md->positions.assign(positions, positions + vertexCount);
		md->normals.assign(normals, normals + vertexCount);
		md->uvs.assign(uvs, uvs + vertexCount);
		md->colors.assign(colors, colors + vertexCount);
		md->tangents.assign(tangents, tangents + vertexCount);
		md->bitangents.assign(bitangents, bitangents + vertexCount);
		md->indices.assign(indices, indices + index_count);

		if (tesselation_level > 0)
		{
			*md = tesselate(*md, tesselation_level, true);
			spherical_uvs(&md->uvs[0], &md->positions[0], md->positions.size());
		}

		if (mb)
			create_buffers(mb,
				md->positions.data(), md->normals.data(), md->uvs.data(), md->colors.data(), md->tangents.data(), md->bitangents.data(), md->positions.size(),
				md->indices.data(), md->indices.size());
	}
	else if (mb) {
		create_buffers(mb, positions, normals, uvs, colors, tangents, bitangents, vertexCount, indices, index_count);
	}

}
void create_frustrum_mesh(MeshBuffers* mb, MeshData* md, f32 near, f32 far, f32 fov)
{
	assert(mb || md);	// one of those for sure, why call it otherwise?
	const tri_idx vertexCount = 8;

	const f32 fov_tan = tan(fov);
	const f32 near_size = fov_tan * near;
	const f32 far_size = fov_tan * far / 2;

	vec3 positions[vertexCount] = {
		// near
		{near_size, near_size, -near},
		{near_size, -near_size, -near},
		{-near_size, -near_size, -near},
		{-near_size, near_size, -near},
		// right
		{far_size, far_size, -far},
		{far_size, -far_size, -far},
		{-far_size, -far_size, -far},
		{-far_size, far_size, -far}
	};
	const f32 half_depth = (far - near) / 2 + near;
	const vec3 center{0, 0, half_depth};
	vec3 normals[vertexCount] = {
		// near
		{glm::normalize(positions[0] - center)},
		{glm::normalize(positions[1] - center)},
		{glm::normalize(positions[2] - center)},
		{glm::normalize(positions[3] - center)},
		// far
		{glm::normalize(positions[4] - center)},
		{glm::normalize(positions[5] - center)},
		{glm::normalize(positions[6] - center)},
		{glm::normalize(positions[7] - center)}
	};
	vec2 uvs[vertexCount] = {
		//front
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f },
		// back
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f }

	};
	Color colors[vertexCount] = { DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR,
								DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR };
	tri_idx indices[36] = {
		// right
		0, 3, 7,
		7, 4, 0,
		// left
		1, 5, 6,
		6, 2, 1,
		// top
		1, 0, 4,
		4, 5, 1,
		// bot
		7, 3, 2,
		2, 6, 7,
		// front
		0, 1, 2,
		2, 3, 0,
		// back
		4, 7, 0,
		0, 5, 4
	};
	// tangent & bitangent computation
	vec3 tangents[vertexCount] = { vec3{} };
	vec3 bitangents[vertexCount] = { vec3{} };

	compute_tangent_basis(tangents, bitangents, positions, normals, uvs, vertexCount, indices, 36);

	if (mb)
		create_buffers(mb, positions, normals, uvs, colors, tangents, bitangents, vertexCount, indices, sizeof(indices) / sizeof(tri_idx));
	if (md) {
		md->positions.assign(positions, positions + vertexCount);
		md->normals.assign(normals, normals + vertexCount);
		md->uvs.assign(uvs, uvs + vertexCount);
		md->colors.assign(colors, colors + vertexCount);
		md->tangents.assign(tangents, tangents + vertexCount);
		md->bitangents.assign(bitangents, bitangents + vertexCount);
		md->indices.assign(indices, indices + 36);
	}

}

void compute_tangent_basis(vec3 * tangents, vec3 * bitangents,
	const vec3 *positions, const vec3 *normals, const vec2 *uvs, const tri_idx vertex_count,
	const tri_idx *indices, const tri_idx index_count)
{
	for (tri_idx i = 0; i < index_count; i += 3) {
		tri_idx i0 = indices[i];
		tri_idx i1 = indices[i + 1];
		tri_idx i2 = indices[i + 2];

		vec3 p0 = positions[i0];
		vec3 p1 = positions[i1];
		vec3 p2 = positions[i2];

		vec3 dPos0 = p1 - p0;
		vec3 dPos1 = p2 - p0;

		vec2 t0 = uvs[i0];
		vec2 t1 = uvs[i1];
		vec2 t2 = uvs[i2];

		vec2 dTex0 = t1 - t0;
		vec2 dTex1 = t2 - t0;

		f32 invDet = 1.f / glm::determinant(mat2(dTex0, dTex1));

		vec3 T = invDet * (dTex1.y * dPos0 - dTex0.y * dPos1);
		vec3 B = invDet * (dTex0.x * dPos1 - dTex1.x * dPos0);

		tangents[i0] += T;
		tangents[i1] += T;
		tangents[i2] += T;

		bitangents[i0] += B;
		bitangents[i1] += B;
		bitangents[i2] += B;
	}

	// Gram-Schmidt
	for (tri_idx i = 0; i < vertex_count; i++) {
		vec3 T = tangents[i];
		vec3 N = normals[i];
		T = T - (glm::dot(T, N) * N);

		vec3 B = bitangents[i];
		B = B - glm::dot(N, B) * N - glm::dot(T, B) * T;
		
		T = glm::normalize(T);
		B = glm::normalize(B);
		//assert(isEqual(glm::length2(T), 1));
		//assert(isEqual(glm::length2(B), 1));
		tangents[i] = T;
		bitangents[i] = B;
	}

}
void compute_tangent_basis(MeshData &md)
{
	assert(md.tangents.empty() && md.bitangents.empty());
	md.tangents = std::vector<vec3>(md.positions.size(), vec3{});
	md.bitangents = std::vector<vec3>(md.positions.size(), vec3{});

	for (size_t i = 0; i < md.indices.size(); i += 3) {
		tri_idx i0 = md.indices[i];
		tri_idx i1 = md.indices[i + 1];
		tri_idx i2 = md.indices[i + 2];

		vec3 p0 = md.positions[i0];
		vec3 p1 = md.positions[i1];
		vec3 p2 = md.positions[i2];

		vec3 dPos0 = p1 - p0;
		vec3 dPos1 = p2 - p0;

		vec2 t0 = md.uvs[i0];
		vec2 t1 = md.uvs[i1];
		vec2 t2 = md.uvs[i2];

		vec2 dTex0 = t1 - t0;
		vec2 dTex1 = t2 - t0;

		f32 invDet = 1.f / glm::determinant(mat2(dTex0, dTex1));

		vec3 T = invDet * (dTex1.y * dPos0 - dTex0.y * dPos1);
		vec3 B = invDet * (dTex0.x * dPos1 - dTex1.x * dPos0);

		md.tangents[i0] += T;
		md.tangents[i1] += T;
		md.tangents[i2] += T;

		md.bitangents[i0] += B;
		md.bitangents[i1] += B;
		md.bitangents[i2] += B;
	}

	// Gram-Schmidt
	for (size_t i = 0; i < md.positions.size(); i++) {
		vec3 T = md.tangents[i];
		vec3 N = md.normals[i];
		T = T - (glm::dot(T, N) * N);

		vec3 B = md.bitangents[i];
		B = B - glm::dot(N, B) * N - glm::dot(T, B) * T;

		T = glm::normalize(T);
		B = glm::normalize(B);
		//assert(isEqual(glm::length2(T), 1));
		//assert(isEqual(glm::length2(B), 1));
		md.tangents[i] = T;
		md.bitangents[i] = B;
	}

}

namespace  // aux namespace
{
	bool parse_face(const std::string& line, int * position_idx, int * uv_idx, int * normal_idx)
	{
		int matches = sscanf_s(line.c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d",
			&position_idx[0], &uv_idx[0], &normal_idx[0],
			&position_idx[1], &uv_idx[1], &normal_idx[1],
			&position_idx[2], &uv_idx[2], &normal_idx[2]
		);
		if (matches == 9)
			return true;
		return false;
	}
	struct Vertex {
		vec3 position, normal;
		vec2 uv;

		bool operator <(const Vertex& rhs) const {
			return std::memcmp((void*)this, (void*)&rhs, sizeof(Vertex)) > 0;
		}
	};
	bool is_vertex_already_in_use(const Vertex& v, const std::map<Vertex, tri_idx> & vertexToOutIndex, tri_idx & result)
	{
		auto it = vertexToOutIndex.find(v);
		if (it == vertexToOutIndex.end())
			return false;	// does not exists, new vertex

		// vertex used, return index
		result = it->second;
		return true;
	}

	MeshData index_vbo(const std::vector<vec3>& positions, const std::vector<vec3>& normals, const std::vector<vec2>& uvs)
	{
		MeshData result;
		std::map<Vertex, tri_idx> vertexToOutIndex;

		//foreach input vertex
		for (tri_idx i = 0; i < positions.size(); i++) {
			Vertex v = { positions[i], normals[i], uvs[i] };
			tri_idx index;
			bool found = is_vertex_already_in_use(v, vertexToOutIndex, index);
			if (found)	// vertex is already in the vbo, take its index
				result.indices.push_back(index);
			else {
				// new vertex, add to output data
				result.positions.push_back(positions[i]);
				result.normals.push_back(normals[i]);
				result.uvs.push_back(uvs[i]);

				tri_idx new_idx = (tri_idx)result.positions.size() - 1;
				result.indices.push_back(new_idx);
				vertexToOutIndex[v] = new_idx;
			}
		}
		return result;
	}

}

bool load_mesh_obj(const char* filepath, MeshBuffers* mb, MeshData* md)
{
	// obj data read
	std::vector<vec3> tmp_positions, tmp_normals;
	std::vector<vec2> tmp_uvs;
	// every combination of vertices to give vbo indexer
	std::vector<vec3> in_positions, in_normals;
	std::vector<vec2> in_uvs;

	//open file
	std::ifstream file(filepath);
	if (!file.is_open())
		return false;

	//read file
	while (!file.eof()) {
		std::string word;
		//read the firest word of the line
		file >> word;

		// attribute cases
		if (word == "v") {
			// position
			vec3 p;
			file >> word;
			p.x = (f32)std::atof(word.c_str());
			file >> word;
			p.y = (f32)std::atof(word.c_str());
			file >> word;
			p.z = (f32)std::atof(word.c_str());

			tmp_positions.push_back(p);

		}
		else if (word == "vt") {
			// uv
			vec2 uv;
			file >> word;
			uv.x = (f32)std::atof(word.c_str());
			file >> word;
			uv.y = (f32)std::atof(word.c_str());

			tmp_uvs.push_back(uv);
		}
		else if (word == "vn") {
			// normal
			vec3 n;
			file >> word;
			n.x = (f32)std::atof(word.c_str());
			file >> word;
			n.y = (f32)std::atof(word.c_str());
			file >> word;
			n.z = (f32)std::atof(word.c_str());

			tmp_normals.push_back(n);
		}
		else if (word == "f") {
			// face
			int p_idx[3], uv_idx[3], n_idx[3];

			//get whole face line
			std::getline(file, word);

			if (!parse_face(word, p_idx, uv_idx, n_idx)) {
				return false;
			}

			// decrement indices
			p_idx[0]--;
			p_idx[1]--;
			p_idx[2]--;
			uv_idx[0]--;
			uv_idx[1]--;
			uv_idx[2]--;
			n_idx[0]--;
			n_idx[1]--;
			n_idx[2]--;

			// create new combination of position, normal, uv
			if (p_idx[0] >= 0) {
				in_positions.push_back(tmp_positions[p_idx[0]]);
				in_positions.push_back(tmp_positions[p_idx[1]]);
				in_positions.push_back(tmp_positions[p_idx[2]]);
			}
			else {
				in_positions.push_back(vec3{});
				in_positions.push_back(vec3{});
				in_positions.push_back(vec3{});
			}
			if (n_idx[0] >= 0) {
				in_normals.push_back(tmp_normals[n_idx[0]]);
				in_normals.push_back(tmp_normals[n_idx[1]]);
				in_normals.push_back(tmp_normals[n_idx[2]]);
			}
			else {
				in_normals.push_back(vec3{});
				in_normals.push_back(vec3{});
				in_normals.push_back(vec3{});
			}
			if (uv_idx[0] >= 0) {
				in_uvs.push_back(tmp_uvs[uv_idx[0]]);
				in_uvs.push_back(tmp_uvs[uv_idx[1]]);
				in_uvs.push_back(tmp_uvs[uv_idx[2]]);
			}
			else {
				in_uvs.push_back(vec2{});
				in_uvs.push_back(vec2{});
				in_uvs.push_back(vec2{});
			}
		}
		else {
			// probably a commnet
			std::getline(file, word);	// skip all the line
		}
	}
	// create the actual final buffers
	MeshData result = index_vbo(in_positions, in_normals, in_uvs);
	// compute tangent basis
	compute_tangent_basis(result);

	// set vertex colors (not read)
	if (result.colors.empty())
		result.colors = std::vector<Color>(result.positions.size(), Color{DEFAULT_COLOR});

	//outputs
	if (mb)
		create_buffers(mb, &result);
	if (md)
		*md = std::move(result);

	return true;
}
bool save_mesh_obj(const char* filepath, const MeshData& md, bool export_tangents)
{
	std::ofstream file(filepath);

	if (!file.is_open())
		return false;

	file << std::fixed << std::setprecision(6);

	std::unordered_map<vec3, tri_idx> exported_positions;
	tri_idx idx = 0;
	for (tri_idx i = 0; i < md.positions.size(); i++) {
		vec3 p = md.positions[i];
		// check if we already exported the position
		auto found = exported_positions.find(p);
		// export if we didnt
		if (found == exported_positions.end()) {
			file << "v ";
			file << p.x << " ";
			file << p.y << " ";
			file << p.z << std::endl;

			exported_positions.insert(std::pair<vec3, tri_idx>(p, idx++));
		}
	}

	std::unordered_map<vec2, tri_idx> exported_uvs;
	idx = 0;
	for (tri_idx i = 0; i < md.uvs.size(); i++) {
		vec2 t = md.uvs[i];
		//check if we already exported the uv
		auto found = exported_uvs.find(t);
		//export if we didnt
		if (found == exported_uvs.end()) {
			file << "vt ";
			file << t.x << " ";
			file << t.y << std::endl;

			exported_uvs.insert(std::pair<vec2, tri_idx>(t, idx++));
		}
	}

	std::unordered_map<vec3, tri_idx> exported_normals;
	idx = 0;
	for (tri_idx i = 0; i < md.normals.size(); i++) {
		vec3 n = md.normals[i];
		// check if we already exported the normal
		auto found = exported_normals.find(n);
		// export if we didnt
		if (found == exported_normals.end()) {
			file << "vn ";
			file << n.x << " ";
			file << n.y << " ";
			file << n.z << std::endl;

			exported_normals.insert(std::pair<vec3, tri_idx>(n, idx++));
		}
	}

	std::unordered_map<vec3, tri_idx> exported_tangents;
	if (export_tangents) {
		idx = 0;
		for (tri_idx i = 0; i < md.tangents.size(); i++) {
			vec3 tan = md.tangents[i];
			// check if we already exported the tangent
			auto found = exported_tangents.find(tan);
			// export if we didnt
			if (found == exported_tangents.end()) {
				file << "vta ";
				file << tan.x << " ";
				file << tan.y << " ";
				file << tan.z << std::endl;

				exported_tangents.insert(std::pair<vec3, tri_idx>(tan, idx++));
			}
		}
	}

	// export all faces
	for (tri_idx i = 0; i < md.indices.size(); i += 3) {
		const vec3 &p0 = md.positions[md.indices[i]];
		const vec3 &p1 = md.positions[md.indices[i + 1]];
		const vec3 &p2 = md.positions[md.indices[i + 2]];

		const vec2 &uv0 = md.uvs[md.indices[i]];
		const vec2 &uv1 = md.uvs[md.indices[i + 1]];
		const vec2 &uv2 = md.uvs[md.indices[i + 2]];

		const vec3 &n0 = md.normals[md.indices[i]];
		const vec3 &n1 = md.normals[md.indices[i + 1]];
		const vec3 &n2 = md.normals[md.indices[i + 2]];

		const vec3 &t0 = md.tangents[md.indices[i]];
		const vec3 &t1 = md.tangents[md.indices[i + 1]];
		const vec3 &t2 = md.tangents[md.indices[i + 2]];

		file << "f ";

		file << (exported_positions.find(p0)->second + 1) << "/";
		file << (exported_uvs.find(uv0)->second + 1) << "/";
		file << (exported_normals.find(n0)->second + 1);
		if (export_tangents)
			file << "/" << (exported_tangents.find(t0)->second + 1);
		file << " ";

		file << (exported_positions.find(p1)->second + 1) << "/";
		file << (exported_uvs.find(uv1)->second + 1) << "/";
		file << (exported_normals.find(n1)->second + 1);
		if (export_tangents)
			file << "/" << (exported_tangents.find(t1)->second + 1);
		file << " ";

		file << (exported_positions.find(p2)->second + 1) << "/";
		file << (exported_uvs.find(uv2)->second + 1) << "/";
		file << (exported_normals.find(n2)->second + 1);
		if (export_tangents)
			file << "/" << (exported_tangents.find(t2)->second + 1);
		file << std::endl;

	}

	return true;
}

MeshData tesselate(const MeshData& md, u8 level, bool normalized_output) 
{
	if (!level)
		return md;

	MeshData res;
	res.positions = md.positions;
	res.normals = md.normals;
	res.uvs = md.uvs;
	res.colors = md.colors;
	res.tangents = md.tangents;
	res.bitangents = md.bitangents;
	res.indices = md.indices;

	auto key = [](tri_idx a, tri_idx b) -> u64 {
		return (static_cast<u64>(a) << 32) | b;
	};
	for (u8 lvl = 0; lvl < level; lvl++) {
		std::unordered_map<u64, tri_idx> edges_interpolated;

		auto interp = [&res, &normalized_output, &edges_interpolated, &key](tri_idx a, tri_idx b) -> tri_idx {
			u64 hash_1 = key(a, b);
			u64 hash_2 = key(b, a);
			if (edges_interpolated.find(hash_1) != edges_interpolated.end() && edges_interpolated.find(hash_2) != edges_interpolated.end())
				return edges_interpolated.at(hash_1);
			vec3 p = (res.positions[a] + res.positions[b]) / 2.f;
			vec3 n = (res.normals[a] + res.normals[b]) / 2.f;
			vec2 t = (res.uvs[a] + res.uvs[b]) / 2.f;
			Color c0 = res.colors[a], c1 = res.colors[b];
			Color c(((u32)c0.r + (u32)c1.r) / 2, ((u32)c0.g + (u32)c1.g) / 2, ((u32)c0.b + (u32)c1.b) / 2, ((u32)c0.a + (u32)c1.a) / 2);
			vec3 tan = (res.tangents[a] + res.tangents[b]) / 2.f;
			vec3 bitan = (res.bitangents[a] + res.bitangents[b]) / 2.f;
			if (normalized_output)
				p = glm::normalize(p);
			res.positions.push_back(p);
			res.normals.push_back(n);
			res.uvs.push_back(t);
			res.colors.push_back(c);
			res.tangents.push_back(tan);
			res.bitangents.push_back(bitan);
			tri_idx index = res.positions.size() - 1;
			edges_interpolated[hash_1] = edges_interpolated[hash_2] = index;
			return index;
		};

		std::vector<tri_idx> new_indices;
		new_indices.reserve(res.indices.size());
		for (tri_idx i = 0; i < res.indices.size(); i += 3) {
			tri_idx i0 = res.indices[i];
			tri_idx i1 = res.indices[i + 1];
			tri_idx i2 = res.indices[i + 2];

			tri_idx a = interp(i0, i1);
			tri_idx b = interp(i1, i2);
			tri_idx c = interp(i2, i0);

			new_indices.push_back(i0); new_indices.push_back(a); new_indices.push_back(c);
			new_indices.push_back(i1); new_indices.push_back(b); new_indices.push_back(a);
			new_indices.push_back(i2); new_indices.push_back(c); new_indices.push_back(b);
			new_indices.push_back(a); new_indices.push_back(b); new_indices.push_back(c);

			//assert(a != i0 && a != i1 && a != i2);
			//assert(b != i0 && b != i1 && b != i2);
			//assert(c != i0 && c != i1 && c != i2);
		}
		res.indices = std::move(new_indices);
	}
	return res;
}

std::vector<TriAdj> adjacent_triangles(tri_idx * indices, const tri_idx index_count)
{
	//output
	std::vector<TriAdj> adjacents(index_count / 3, std::array<int, 3>{-1, -1, -1});
	
	// auxiliary triangle struct
	struct Tri {
		tri_idx a, b, c;
		std::array<tri_idx, 3> sorted_idx;
		Tri(tri_idx a, tri_idx b, tri_idx c)
			: a(a), b(b), c(c), sorted_idx({ a, b, c })
		{
			std::sort(sorted_idx.begin(), sorted_idx.end());
		}
		bool operator<(const Tri& rhs)const {
			if (sorted_idx[2] < rhs.sorted_idx[2])
				return true;
			if (sorted_idx[2] == rhs.sorted_idx[2]) {
				if (sorted_idx[1] < rhs.sorted_idx[1])
					return true;
				if (sorted_idx[1] == rhs.sorted_idx[1])
					if (sorted_idx[0] < rhs.sorted_idx[0])
						return true;
			}
			return false;
		}
		bool operator==(const Tri& rhs)const {
			return sorted_idx[0] == rhs.sorted_idx[0]
				&& sorted_idx[1] == rhs.sorted_idx[1]
				&& sorted_idx[2] == rhs.sorted_idx[2];
		}
	};
	//odered input (modifies original input to this one, so adjacency list matches indices)
	std::set<Tri> ordered_triangles;

	//must define a way to hash pairs
#if 0	// this is more readable for debugging
	auto key = [](tri_idx a, tri_idx b) -> std::string {
		return std::to_string(a) + "," + std::to_string(b);
	};
	std::unordered_map<std::string, u16> edge_adjacency;
#else
	auto key = [](tri_idx a, tri_idx b) -> u64 {
		return (static_cast<u64>(a) << 32) | b;
	};
	std::unordered_map<u64, tri_idx> edge_adjacency;
#endif

	//loop for avery triplet adding edge/3rd to the map
	tri_idx i;
	for (i = 0; i < index_count; i += 3) {
		tri_idx a = indices[i], b = indices[i + 1], c = indices[i + 2];
		//insert sorted triangle
		Tri sorted_tri{ a, b, c };
		ordered_triangles.insert(sorted_tri);
		//fill edge map	(always same edge direction)
		edge_adjacency[key(a, b)] = c;
		edge_adjacency[key(b, c)] = a;
		edge_adjacency[key(c, a)] = b;
	}
	//transform input triangles to ordered triangles
	i = 0;
	for (const Tri &t : ordered_triangles) {
		indices[i++] = t.a;
		indices[i++] = t.b;
		indices[i++] = t.c;
	}
	assert(i == index_count);
	//fill adjacency list
	for (i = 0; i < index_count; i += 3) {
		tri_idx a = indices[i], b = indices[i + 1], c = indices[i + 2];
		auto & adj = adjacents[i / 3];
		//t0
		auto third_vtx = edge_adjacency.find(key(c, b));
		//assert(third_vtx != edge_adjacency.end());
		if (third_vtx != edge_adjacency.end()) {
			const auto &adj_it = ordered_triangles.find(Tri{ b, c, third_vtx->second });
			assert(adj_it != ordered_triangles.end());
			tri_idx idx = std::distance(ordered_triangles.begin(), adj_it);
			assert(idx != i / 3);
			adj[0] = idx;
		}
		//t1
		third_vtx = edge_adjacency.find(key(a, c));
		//assert(third_vtx != edge_adjacency.end());
		if (third_vtx != edge_adjacency.end()) {
			const auto &adj_it = ordered_triangles.find(Tri{ c, a, third_vtx->second });
			assert(adj_it != ordered_triangles.end());
			tri_idx idx = std::distance(ordered_triangles.begin(), adj_it);
			assert(idx != i / 3);
			adj[1] = idx;
		}
		//t2
		third_vtx = edge_adjacency.find(key(b, a));
		//assert(third_vtx != edge_adjacency.end());
		if (third_vtx != edge_adjacency.end()) {
			const auto &adj_it = ordered_triangles.find(Tri{ a, b, third_vtx->second });
			assert(adj_it != ordered_triangles.end());
			tri_idx idx = std::distance(ordered_triangles.begin(), adj_it);
			assert(idx != i / 3);
			adj[2] = idx;
		}
	}
	return adjacents;
}

void draw_voronoi_graph(vec3 const* positions, 
	tri_idx const *indices, 
	TriAdj const* triangle_adjacency, tri_idx triangle_count, Color color)
{
	for (tri_idx i = 0; i < triangle_count; i++) {
		std::array<tri_idx, 3> tri = { indices[i * 3], indices[i * 3 + 1], indices[i * 3 + 2] };
		auto circle = get_circumcircle(positions[tri[0]], positions[tri[1]], positions[tri[2]]);
		for (int j = 0; j < 3; j++) {
			int adj = triangle_adjacency[i][j];
#if 0
			if (adj == -1) {
				//find the oposite edge
				int i0 = -1, i1 = -1;
				switch (j) {
				case 0:
					i0 = tri[1];
					i1 = tri[2];
					break;
				case 1:
					i0 = tri[2];
					i1 = tri[0];
					break;
				case 2:
					i0 = tri[1];
					i1 = tri[0];
					break;
				}
				vec2 v0 = points[i0], v1 = points[i1];
				vec2 v2 = { 1.f, 1.f };
				//find closest corner
				{
					f32 closest_dist = glm::length2(circle.first - vec2{ 1.f, 1.f });
					f32 tmp = glm::length2(circle.first - vec2{ -1.f, 1.f });
					if (tmp < closest_dist) {
						closest_dist = tmp;
						v2 = { -1.f, 1.f };
					}
					tmp = glm::length2(circle.first - vec2{ -1.f, -1.f });
					if (tmp < closest_dist) {
						closest_dist = tmp;
						v2 = { -1.f, -1.f };
					}
					tmp = glm::length2(circle.first - vec2{ 1.f, -1.f });
					if (tmp < closest_dist) {
						closest_dist = tmp;
						v2 = { 1.f, -1.f };
					}
				}
				auto corner_circle = get_circumcircle(v0, v1, v2);
				vec2 dif = corner_circle.first - circle.first;
				if (glm::length2(circle.first) > glm::length2(circle.first + dif))
					dif = -dif;
				graphics.draw_line(circle.first, circle.first + dif * 10.f, color, color);
			}
#endif		
			if (adj > (int)i) {
				auto adj_circle = get_circumcircle(positions[indices[adj * 3]], positions[indices[adj * 3 + 1]], positions[indices[adj * 3 +2]]);
				graphics.draw_line(circle.first, adj_circle.first, color, color);
			}
		}

	}
}


//create an EdgeGraph from a list of triangles
EdgeGraph edge_graph(tri_idx const *indices, tri_idx tri_count, tri_idx vertex_count)
{
	EdgeGraph edges(vertex_count);
	for (tri_idx i = 0; i < tri_count; i += 3) {
		tri_idx i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
		edges[i0].insert(i1);
		edges[i0].insert(i2);

		edges[i1].insert(i0);
		edges[i1].insert(i2);

		edges[i2].insert(i0);
		edges[i2].insert(i1);
	}
	return edges;
}