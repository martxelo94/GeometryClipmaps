/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: camera.h
Purpose: define camera's properties an easy matrix acces
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include "math_utils.h"

struct Camera
{
	vec3 pos;
	vec3 target;	// where to look at
	vec2 size = {1, 1};	// screen width and height
	f32 near = 0.1f, far = 1000.f;
	f32 fov = f32(DEG_TO_RAD * 60);

	__forceinline mat4 get_view(vec3 up = vec3{0, 1, 0}) const { return glm::lookAt(pos, target, up); }
	__forceinline mat4 get_proj() const { return glm::perspective(fov, size.x / size.y, near, far); }
	__forceinline mat4 ortho_proj() const { vec2 hs = size / 2.f; return glm::orthoRH(-hs.x, hs.x, -hs.y, hs.y, near, far); }
	__forceinline vec3 ndc_to_world(vec3 ndc_coords) const {
		vec4 world_pos = glm::inverse(get_proj() * get_view()) * vec4{ ndc_coords, 1 };
		world_pos.x /= world_pos.w; world_pos.y /= world_pos.w; world_pos.z /= world_pos.w;
		return world_pos;
	}
};