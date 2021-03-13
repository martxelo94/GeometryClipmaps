#include "model.h"
#include "assimp\postprocess.h"
#include "gtc\type_ptr.hpp"
#include "assimp\cimport.h"
#include "material.h"

#include <iostream>

void load_assimp_texture(aiString path) 
{
	std::string tmp(".");
	tmp.append(path.C_Str());
	path = tmp;
	size_t slash_pos = tmp.find_last_of('/');
	tmp = tmp.substr(slash_pos + 1);
	GLuint & tex_handler = graphics.texture_map[tmp];
	if (tex_handler == 0)
		load_texture(&tex_handler, path.C_Str(), true);	// assimp inverts Y coord...
}

Model::Model(const char* filepath)
{
	this->scene = aiImportFile(filepath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	//create buffers
	meshes.reserve(scene->mNumMeshes);
	for (u32 i = 0; i < scene->mNumMeshes; i++)
		meshes.push_back(MeshBuffers(*scene->mMeshes[i]));

	
	//load material textures
	for (u32 i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial *mat = scene->mMaterials[i];
		u32 texture_count = mat->GetTextureCount(aiTextureType_DIFFUSE);
		// diffuse textures
		for (u32 i = 0; i < texture_count; i++) {
			aiString tex_path;
			mat->GetTexture(aiTextureType_DIFFUSE, i, &tex_path);
			load_assimp_texture(tex_path);
		}
		// normal map textures
		texture_count = mat->GetTextureCount(aiTextureType_HEIGHT);
		for (u32 i = 0; i < texture_count; i++) {
			aiString tex_path;
			mat->GetTexture(aiTextureType_HEIGHT, i, &tex_path);
			load_assimp_texture(tex_path);
		}
		// specular textures
		texture_count = mat->GetTextureCount(aiTextureType_SPECULAR);
		for (u32 i = 0; i < texture_count; i++) {
			aiString tex_path;
			mat->GetTexture(aiTextureType_SPECULAR, i, &tex_path);
			load_assimp_texture(tex_path);
		}
	}
}

Model::Model(Model && other)
{
	// transfer mesh buffers
	meshes = std::move(other.meshes);
	other.meshes.clear();
	// transfer scene data, delete old
	if (scene)
		delete scene;
	scene = other.scene;
	other.scene = nullptr;
	// transfer name
	name = std::move(other.name);
	other.name.clear();
}
Model& Model::operator=(Model &&other)
{
	// transfer mesh buffers
	meshes = std::move(other.meshes);
	other.meshes.clear();
	// transfer scene data, delete old
	if (scene)
		delete scene;
	scene = other.scene;
	other.scene = nullptr;
	// transfer name
	name = std::move(other.name);
	other.name.clear();
	return *this;
}
Model::~Model()
{
	aiReleaseImport(scene);
}

void Model::draw(Shader sh) const
{
	// stuff common for all meshes


	recursive_draw(scene->mRootNode, mat4(), sh);
}

void Model::recursive_draw(aiNode* node, mat4 model_mtx, Shader sh) const
{
	// compute model matrix
	mat4 node_mtx = glm::make_mat4(node->mTransformation.Transpose()[0]);
	model_mtx = model_mtx * node_mtx;
	// bind uniforms
	glUniformMatrix4fv(glGetUniformLocation(graphics.shader_program[sh], "MODEL"), 1, false, &(model_mtx[0][0]));



	// draw meshes
	for (u32 i = 0; i < node->mNumMeshes; i++) {
		const GLuint mesh_idx = node->mMeshes[i];
		glBindVertexArray(meshes[mesh_idx].vao);
		//bind material
		aiMaterial * material = scene->mMaterials[scene->mMeshes[mesh_idx]->mMaterialIndex];
		set_uniforms(sh, material);
		
		// draw elements
		glDrawElements(GL_TRIANGLES, meshes[mesh_idx].index_count, GL_UNSIGNED_INT, 0);
	}

	// draw children
	for (u32 i = 0; i < node->mNumChildren; i++)
		recursive_draw(node->mChildren[i], model_mtx, sh);
}
