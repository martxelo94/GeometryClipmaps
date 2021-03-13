/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: material.h
Purpose: implement Material type
Language: c++
Platform: windows 10
Project: cs300_markel.p_1
Author: Markel Pisano Berrojalbiz
Creation date: 6/1/2019
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "graphics.h"

/*
	ON SHADER:

	struct Material {
		sampler2D texture;
		vec3 diffuse;
		vec3 specular;
		float shininess;
		float shine_intensity;
	};
	uniform Material material;
*/

struct Material
{
	GLuint * t_diffuse = &graphics.tex[Texture::t_white];	// pointer to texture
	GLuint * t_normals = nullptr;
	GLuint * t_specular = nullptr;
	Color diffuse, specular;
	f32 shininess = 32.f;
	f32 shine_intensity = 1.f;

	void edit(const char * node_name);

	__forceinline void set_uniforms(Shader sh_program) const {
		assert(t_diffuse);
		int uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_diffuse");
		glUniform1i(uniform_loc, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *t_diffuse);
		
		if (t_normals) {
			uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_normals");
			glUniform1i(uniform_loc, 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *t_normals);
		}
		if (t_specular) {
			uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.t_specular");
			glUniform1i(uniform_loc, 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, *t_specular);
		}


		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.diffuse");
		diffuse.set_uniform_RGB(uniform_loc);

		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.specular");
		specular.set_uniform_RGB(uniform_loc);

		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.shininess");
		glUniform1f(uniform_loc, shininess);

		uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "material.shine_intensity");
		glUniform1f(uniform_loc, shine_intensity);
	}
};

#include "assimp\material.h"
#ifdef AI_MATERIAL_H_INC
void set_uniforms(Shader sh_program, const aiMaterial *mat);
#endif