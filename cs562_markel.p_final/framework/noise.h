#pragma once

#include "math_utils.h"
#include "color.h"	// Color
#include <vector>

namespace Noise
{
	using Method = f32(*)(vec2, f32);

	std::vector<f32> create_depth_texture(Method noise_method, u32 size, vec2 offset, f32 freq, f32 lacunarity, f32 persistence, u8 oct);
	std::vector<Color> create_color_texture(Method noise_method, u32 size, vec2 offset, f32 freq, f32 lacunarity, f32 persistence, u8 oct);
	std::vector<Color> depth_to_gradient_color(const std::vector<f32> &depth);

	// noise methods
	f32 perlin2D(vec2 point, f32 freq);
	f32 value2D(vec2 point, f32 freq);

};