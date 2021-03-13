#include "material.h"

void Material::edit(const char * node_name)
{
	if (ImGui::TreeNode(node_name)) {
		vec3 tmp(f32(diffuse.r) / 255, f32(diffuse.g) / 255, f32(diffuse.b) / 255);
		ImGui::ColorEdit3("Diffuse", &tmp[0]);
		diffuse = Color{ vec4{ tmp, 1.f } };
		tmp = vec3(f32(specular.r) / 255, f32(specular.g) / 255, f32(specular.b) / 255);
		ImGui::ColorEdit3("Specular", &tmp[0]);
		specular = Color{ vec4{ tmp, 1.f } };

		ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.0, 1000.f);
		ImGui::DragFloat("Shine Intensity", &shine_intensity, 0.1f, 0.0, 1000.f);
		ImGui::TreePop();
	}

}


std::string get_texture_name(const aiString &path)
{
	std::string tmp(path.C_Str());
	size_t slash_pos = tmp.find_last_of('/');
	tmp = tmp.substr(slash_pos + 1);
	return tmp;
}

void set_uniforms(Shader sh_program, const aiMaterial *mat)
{
	int uniform_loc;
	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_diffuse");
		glUniform1i(uniform_loc, 0);
		glActiveTexture(GL_TEXTURE0);
		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		const GLuint texture = graphics.texture_map[get_texture_name(path)];
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {

		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_normals");
		glUniform1i(uniform_loc, 1);
		glActiveTexture(GL_TEXTURE1);
		aiString path;
		mat->GetTexture(aiTextureType_HEIGHT, 0, &path);
		const GLuint normal_map = graphics.texture_map[get_texture_name(path)];
		glBindTexture(GL_TEXTURE_2D, normal_map);

	}
	if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_specular");
		glUniform1i(uniform_loc, 1);
		glActiveTexture(GL_TEXTURE2);
		aiString path;
		mat->GetTexture(aiTextureType_SPECULAR, 0, &path);
		const GLuint normal_map = graphics.texture_map[get_texture_name(path)];
		glBindTexture(GL_TEXTURE_2D, normal_map);
	}

	Color color;
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.diffuse");
	aiColor4D c;
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &c)) {
		color = Color{ vec4{ c.r, c.g, c.b, c.a } };
	}
	color.set_uniform_RGB(uniform_loc);

	// AMBIENT
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.ambient");
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &c)) {
		color = Color{ vec4{ c.r, c.g, c.b, c.a } };
	}
	else
		color = Color{};
	color.set_uniform_RGB(uniform_loc);
	// SPECULAR
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.specular");
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &c)) {
		color = Color{ vec4{ c.r, c.g, c.b, c.a } };
	}
	else
		color = Color{};
	color.set_uniform_RGB(uniform_loc);
	// EMISSIVE
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.emissive");
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &c)) {
		color = Color{ vec4{ c.r, c.g, c.b, c.a } };
	}
	else
		color = Color{};
	color.set_uniform_RGB(uniform_loc);

	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.shininess");
	f32 shininess;
	u32 max;
	aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max);
	glUniform1f(uniform_loc, shininess);

	//uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.shine_intensity");
	//glUniform1f(uniform_loc, shine_intensity);
}
