/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: light.h
Purpose: implement every light type
Language: c++
Platform: windows 10
Project: cs300_markel.p_1
Author: Markel Pisano Berrojalbiz
Creation date: 6/1/2019
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "color.h"
#include "graphics.h"


/*
	LIGHT TYPE ATTRIBUTES

	-ambient		(1): diffuse color
	-directional	(2): diffuse color, specular, direction
	-point			(3): diffuse color, specular, position
	-spot			(7): diffuse color, specular, position, direction, small angle, big angle, falloff
*/

/* Interface for every Light type */
struct Light
{
	Color diffuse;
	virtual void set_uniforms(Shader sh_program, mat4 const* view_mtx = nullptr, u8 light_idx = 0) const = 0;
	// implement ImGui light editor
	virtual void edit(const char * node_name) = 0;
};

struct AmbientLight : public Light
{
	/*
	Only ONE of this per shader!
	vec3 LAmbient;	// ambient color on shader
*/

	inline void set_uniforms(Shader sh_program, mat4 const* view_mtx = nullptr, u8 light_idx = 0) const;
	void edit(const char * node_name) override;
};

struct DirectionalLight : public Light
{
	/*
	ON SHADER:

	struct DirectionalLight{
		vec3 diffuse;
		vec3 specular;
		vec3 direction;	// camera space!
	};
	uniform DirectionalLight LDirectional[MAX_LIGHT_COUNT];
	uniform int LDirectional_count;
*/
	Color specular;
	vec3 direction;

	inline void set_uniforms(Shader sh_program, mat4 const* view_mtx = nullptr, u8 light_idx = 0) const;
	void edit(const char * node_name) override;
};

struct PointLight : public Light
{
	/*
	ON SHADER:

	struct PointLight{
		vec3 diffuse;
		vec3 specular;
		vec3 position;	// camera space!
		vec3 attenuation;	// x: constant, y: linear, z: quadratic
	};
	uniform PointLight LPoint[MAX_LIGHT_COUNT];
	uniform int LPoint_count;
	(IDENTICAL ATTRIBUTES TO DIRECTIONAL...)
*/
	Color specular;
	vec3 position;
	vec3 attenuation = {0, 0, 1};
	inline void set_uniforms(Shader sh_program, mat4 const* view_mtx = nullptr, u8 light_idx = 0) const;
	void edit(const char * node_name) override;
};
struct SpotLight : public Light
{
	/*
	ON SHADER:

	struct SpotLight{
		vec3 diffuse;
		vec3 specular;
		vec3 position;	// camera space!
		vec3 direction;	// camera space!
		vec3 attenuation;	// x: constant, y: linear, z: quadratic
		float angle0;	// small angle RADIANS
		float angle1;	// big angle   RADIANS
		float falloff;
	};
	uniform SpotLight LSpot[MAX_LIGHT_COUNT];
	uniform int LSpot_count;
*/

	Color specular;
	vec3 position, direction;
	vec3 attenuation = {1, 1, 1};
	f32 angle0    = 70	// DEGREES
		, angle1  = 90	// DEGREES
		, falloff = 1.f;
	inline void set_uniforms(Shader sh_program, mat4 const* view_mtx = nullptr, u8 light_idx = 0) const;
	void edit(const char * node_name) override;
	inline mat4 get_view() const { return glm::lookAt(position, position + direction, vec3{ 0, 1, 0 }); }
	inline mat4 get_proj() const { return glm::perspective(glm::radians(angle1), 1.f, 0.1f, 50.f); }
};

void AmbientLight::set_uniforms(Shader sh_program, mat4 const*, u8) const {
	//ignore view_mtx and light_idx

	int uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], "LAmbient");
	diffuse.set_uniform_RGB(uniform_loc);

	pop_gl_errors(__FUNCTION__);
}
void DirectionalLight::set_uniforms(Shader sh_program, mat4 const* view_mtx, u8 light_idx) const {
	const std::string base_name = "LDirectional[" + std::to_string(light_idx) + "].";
	std::string uniform_name = base_name + "diffuse";
	int uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	diffuse.set_uniform_RGB(uniform_loc);
	
	uniform_name = base_name + "specular";
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	specular.set_uniform_RGB(uniform_loc);

	assert(view_mtx);
	uniform_name = base_name + "direction";
	uniform_loc = glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	vec4 p = *view_mtx * vec4(direction, 0);
	glUniform3fv(uniform_loc, 1, &p[0]);

	pop_gl_errors(__FUNCTION__);
}
void PointLight::set_uniforms(Shader sh_program, mat4 const* view_mtx, u8 light_idx) const {
	const std::string base_name = "LPoint[" + std::to_string(light_idx) + "].";
	std::string uniform_name = base_name + "diffuse";
	int uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	diffuse.set_uniform_RGB(uniform_loc);

	uniform_name = base_name + "specular";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	specular.set_uniform_RGB(uniform_loc);

	assert(view_mtx);
	uniform_name = base_name + "position";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	vec4 p = *view_mtx * vec4(position, 1);
	glUniform3fv(uniform_loc, 1, &p[0]);

	uniform_name = base_name + "attenuation";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	glUniform3fv(uniform_loc, 1, &attenuation[0]);

	pop_gl_errors(__FUNCTION__);
}
void SpotLight::set_uniforms(Shader sh_program, mat4 const* view_mtx, u8 light_idx) const {
	const std::string base_name = "LSpot[" + std::to_string(light_idx) + "].";
	std::string uniform_name = base_name + "diffuse";
	int uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	diffuse.set_uniform_RGB(uniform_loc);

	uniform_name = base_name + "specular";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	specular.set_uniform_RGB(uniform_loc);

	assert(view_mtx);
	uniform_name = base_name + "position";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	vec4 p = *view_mtx * vec4(position, 1);
	glUniform3fv(uniform_loc, 1, &p[0]);
	
	uniform_name = base_name + "direction";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	p = *view_mtx * vec4(direction, 0);
	glUniform3fv(uniform_loc, 1, &p[0]);

	uniform_name = base_name + "attenuation";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	glUniform3fv(uniform_loc, 1, &attenuation[0]);

	uniform_name = base_name + "angle0";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	glUniform1f(uniform_loc, glm::radians(angle0));

	uniform_name = base_name + "angle1";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	glUniform1f(uniform_loc, glm::radians(angle1));

	uniform_name = base_name + "falloff";
	uniform_loc =  glGetUniformLocation(graphics.shader_program[sh_program], uniform_name.c_str());
	glUniform1f(uniform_loc, falloff);

	pop_gl_errors(__FUNCTION__);
}

struct Camera;

void light_scissor_optimization(const PointLight& light, f32 min_attenuation, const Camera& camera, bool show_scissor_test = false);
