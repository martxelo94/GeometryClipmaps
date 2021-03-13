#pragma once

#include "assimp/scene.h"
#include "mesh.h"
#include "graphics.h"	// shader type

/*
	Model structure stores GPU buffer data of a loaded aiScene
	aiScene might be kept aswell for model data in CPU
	Model is a unique resource
*/

struct Model
{

	Model() = default;
	Model(const Model &) = delete;
	Model(const char* filepath);
	Model(Model && other);
	Model &operator=(Model &&other);
	~Model();

	void draw(Shader sh) const;
	void recursive_draw(aiNode* node, mat4 model_mtx, Shader sh) const;

	// meshes
	std::vector<MeshBuffers> meshes;
	// materials
	// animations?
	const aiScene *scene = nullptr;
	std::string name;
};
