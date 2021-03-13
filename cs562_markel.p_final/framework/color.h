/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: color.h
Purpose: define color's properties and easy shader binding
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"	// basic types
#include <glew.h>	// glUniform
#include <SDL_endian.h>

#define DEFAULT_COLOR 0xffffffff	// white

// CHECK ENDIANNES!!!
#define IS_BIG_ENDIAN 

struct Color {
	union {
		u32 c;
		struct {
			u8 r, g, b, a;
		};
		u8 v[4];
	};
	Color() 
		: c(0xFFFFFFFF)	// white default
	{}
	Color(u8 r, u8 g, u8 b, u8 a = 255)
		: r(r), g(g), b(b), a(a)
	{}
	Color(u32 color)
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		: Color(u8((color & 0xFF000000) >> 24), u8((color & 0x00FF0000) >> 16), u8((color & 0x0000FF00) >> 8), u8(color & 0x000000FF))
#else
		: Color(u8(color & 0x000000FF), u8((color & 0x0000FF00) >> 8), u8((color & 0x00FF0000) >> 16), u8((color & 0xFF000000) >> 24))
#endif
	{}
	Color(const vec4 &color)
		: r((u8)(color.r * 255)), g((u8)(color.g * 255)), b((u8)(color.b * 255)), a((u8)(color.a * 255))
	{}
	__forceinline explicit operator vec4() const {
		return vec4{ r, g, b, a } / 255.f;
	}
	//uniform set (convert from [0, 255] to [0, 1])
	__forceinline void set_uniform_RGBA(int uniform_location) const {
		glUniform4f(uniform_location, (f32)r / 255, (f32)g / 255, (f32)b / 255, (f32)a / 255);
	}
	__forceinline void set_uniform_RGB(int uniform_location) const {
		glUniform3f(uniform_location, (f32)r / 255, (f32)g / 255, (f32)b / 255);
	}
	// dont multiply alpha channel
	Color operator*(const f32& rhs) const { return Color{ (u8)(r * rhs), (u8)(g * rhs), (u8)(b * rhs), a }; }
	Color& operator*=(const f32& rhs) { return *this = Color{ (u8)(r * rhs), (u8)(g * rhs), (u8)(b * rhs), a }; }
	// dont divide alpha channel
	Color operator/(const f32& rhs) const { return Color{ (u8)(r / rhs), (u8)(g / rhs), (u8)(b / rhs), a }; }
	Color& operator/=(const f32& rhs) { return *this = Color{ (u8)(r / rhs), (u8)(g / rhs), (u8)(b / rhs), a }; }
};
