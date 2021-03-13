#include "noise.h"

namespace Noise
{
	namespace
	{
#define HASH_MASK 255
		// this hash is for PerlinNoise
		int hash[(HASH_MASK + 1) * 2] = {
			151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
			140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
			247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
			57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
			74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
			60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
			65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
			200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
			52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
			207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
			119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
			129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
			218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
			81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
			184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
			222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180,
			//repeated for the second hash
			151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
			140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
			247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
			57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
			74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
			60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
			65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
			200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
			52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
			207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
			119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
			129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
			218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
			81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
			184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
			222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
		};
		f32 smooth(f32 t) {
			return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
			//return t;
		}
		f32 sum(Method noise_method, vec2 point, f32 freq, f32 lacunarity, f32 persistence, u8 oct) {
			f32 sum = noise_method(point, freq);
			f32 amplitude = 1.f;
			f32 range = 1.f;
			for (int o = 1; o < oct; o++) {
				freq *= lacunarity;
				amplitude *= persistence;
				range += amplitude;
				sum += noise_method(point, freq) * amplitude;
			}
			sum = sum / range;
			if (!_finite(sum)) {
				printf("\rERROR: Inf. sum!");
				return 0.0f;
			}
			return sum;
		}

		// DEFINE COLOR GRADIENT
		using Gradient = std::pair<Color, f32>;
		Gradient color_gradient[] = {
			{0x0000ffff, 0.f}, {0xffff00ff, 0.5f}, {0x00ff00ff, 0.6f}, {0xffffffff, 0.8f}
		};
		//Gradient color_gradient[] = {
		//	{0x000000ff, 0.f}, {0xffffffff, 0.5f}
		//};
#if 0	// with interpolation
		Color pick_color(f32 s) {
			int prev = -1;
			int i;
			for (i = 0; i < sizeof(color_gradient) / sizeof(Gradient); i++) {
				if (s < color_gradient[i].second)
					break;
				prev = i;
			}
			assert(prev >= 0);
			Color c0 = color_gradient[prev].first;
			Color c1 = color_gradient[i].first;
			vec4 vColor0 = vec4{ c0.r, c0.g, c0.b, c0.a } / 255.f;
			vec4 vColor1 = vec4{ c1.r, c1.g, c1.b, c1.a } / 255.f;
			f32 sdiff = color_gradient[i].second - color_gradient[prev].second;
			f32 t = (s - color_gradient[prev].second) / sdiff;
			assert(t <= 1 && t >= 0);
			vColor0 = glm::mix(vColor0, vColor1, t);
			vColor0.a = 1;
			return Color{ vColor0 };
		};
#else	// without interpolation
		Color pick_color(f32 s) {
			int i;
			for (i = 1; i < sizeof(color_gradient) / sizeof(Gradient); i++) {
				if (s < color_gradient[i].second)
					break;
			}
			return color_gradient[i - 1].first;
		};
#endif
	}



	std::vector<f32> create_depth_texture(Method noise_method, u32 size, vec2 offset, f32 freq, f32 lacunarity, f32 persistence, u8 oct)
	{
		//create texture pixels
		std::vector<f32> tex(size * size);
		const f32 step = 1.f / size;

		
		vec2 p00 = vec2{ -0.5f, -0.5f};
		vec2 p10 = vec2{ 0.5f, -0.5f};
		vec2 p01 = vec2{ -0.5f, 0.5f};
		vec2 p11 = vec2{ 0.5f, 0.5f};

		for (u32 y = 0; y < size; y++) {
			f32 t = (y + 0.5f) * step + offset.y;
			vec2 p0 = glm::mix(p00, p01, t);
			vec2 p1 = glm::mix(p10, p11, t);
			for (u32 x = 0; x < size; x++) {
				t = (x + 0.5f) * step + offset.x;
				vec2 p = glm::mix(p0, p1, t);
				f32 sample = sum(noise_method, p, freq, lacunarity, persistence, oct);
				//clamp
				sample = (sample + 1) / 2;
				sample = glm::clamp(sample, 0.f, 1.f);
				assert(sample >= 0 && sample <= 1);
				//if (sample < 0)
				//	assert(0);
				
				//set pixel
				u32 idx = y * size + x;
				tex[idx] = sample;
			}
		}

		return tex;
	}
	std::vector<Color> create_color_texture(Method noise_method, u32 size, vec2 offset, f32 freq, f32 lacunarity, f32 persistence, u8 oct)
	{
		//create texture pixels
		std::vector<Color> tex(size * size);

		vec2 p00 = vec2{ -0.5f, -0.5f };
		vec2 p10 = vec2{ 0.5f, -0.5f };
		vec2 p01 = vec2{ -0.5f, 0.5f };
		vec2 p11 = vec2{ 0.5f, 0.5f };

		const f32 step = 1.f / size;
		for (u32 y = 0; y < size; y++) {
			f32 t = (y + 0.5f) * step + offset.y;
			vec2 p0 = glm::mix(p00, p01, t);
			vec2 p1 = glm::mix(p10, p11, t);
			for (u32 x = 0; x < size; x++) {
				t = (x + 0.5f) * step + offset.x;
				vec2 p = glm::mix(p0, p1, t);
				f32 sample = sum(noise_method, p, freq, lacunarity, persistence, oct);
				//clamp
				sample = (sample + 1) / 2;
				sample = glm::clamp(sample, 0.f, 1.f);
				assert(sample >= 0 && sample <= 1);
				//if (sample < 0)
				//	assert(0);

				//set pixel
				u32 idx = y * size + x;
				//tex[idx] = Color{ vec4{sample, sample, sample, 1.f} }; // pick_color(sample);
				tex[idx] = pick_color(sample);
			}
		}
		return tex;
	}
	std::vector<Color> depth_to_gradient_color(const std::vector<f32> &depth)
	{
		std::vector<Color> res;
		res.reserve(depth.size());
		for (size_t i = 0; i < depth.size(); i++) 
			res.push_back(pick_color(depth[i]));
			//res.push_back(pick_color((f32)depth[i] / 255));
		return res;
	}

	// noise methods
	f32 perlin2D(vec2 point, f32 freq)
	{
		point *= freq;
		int ix0 = (int)glm::floor(point.x);
		int iy0 = (int)glm::floor(point.y);
		f32 tx0 = point.x - ix0;
		f32 ty0 = point.y - iy0;
		f32 tx1 = tx0 - 1.f;
		f32 ty1 = ty0 - 1.f;
		ix0 &= HASH_MASK;
		iy0 &= HASH_MASK;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;

		int h0 = hash[ix0], h1 = hash[ix1];
		//define gradients
		static const vec2 gradients[8] = {
			vec2{1, 0}, vec2{-1, 0}, vec2{0, 1}, vec2{0, -1},
			glm::normalize(vec2{1, 1}), glm::normalize(vec2{-1, 1}),
			glm::normalize(vec2{1, -1}), glm::normalize(vec2{-1, -1})
		};
		const int gradientMask = 7;
		vec2 g00 = gradients[hash[h0 + iy0] & gradientMask];
		vec2 g10 = gradients[hash[h1 + iy0] & gradientMask];
		vec2 g01 = gradients[hash[h0 + iy1] & gradientMask];
		vec2 g11 = gradients[hash[h1 + iy1] & gradientMask];

		f32 v00 = glm::dot(g00, vec2{ tx0, ty0 });
		f32 v10 = glm::dot(g10, vec2{ tx1, ty0 });
		f32 v01 = glm::dot(g01, vec2{ tx0, ty1 });
		f32 v11 = glm::dot(g11, vec2{ tx1, ty1 });

		tx0 = smooth(tx0);
		ty0 = smooth(ty0);

		return glm::mix(glm::mix(v00, v10, tx0), glm::mix(v01, v11, tx0), ty0) * (f32)sqrt(2);
	}
	f32 value2D(vec2 point, f32 freq)
	{
		point *= freq;
		int ix0 = (int)glm::floor(point.x);
		int iy0 = (int)glm::floor(point.y);
		f32 tx = point.x - ix0;
		f32 ty = point.y - iy0;
		ix0 &= HASH_MASK;
		iy0 &= HASH_MASK;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;

		int h0 = hash[ix0], h1 = hash[ix1];
		int h00 = hash[h0 + iy0];
		int h10 = hash[h1 + iy0];
		int h01 = hash[h0 + iy1];
		int h11 = hash[h1 + iy1];

		tx = smooth(tx);
		ty = smooth(ty);

		return glm::mix(glm::mix(h00, h10, tx), glm::mix(h01, h11, tx), ty) * (1.f / HASH_MASK);

	}
}