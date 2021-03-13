		#version 420
        
		layout(location = 0) out vec3 gPosition;
		layout(location = 1) out vec4 gNormalShininess;
		layout(location = 2) out vec4 gAlbedo;
		
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 normal;
		layout(location = 2) in vec2 uv;
        layout(location = 3) in vec4 color;
		layout(location = 4) in vec3 tangent;
		layout(location = 5) in vec3 bitangent;

		layout(location = 6) in vec3 world_position;
        
    struct Material {
        sampler2D t_diffuse;
		sampler2D t_normals;
        sampler2D t_specular;
		float shininess;
	};

		uniform Material material;     

		uniform float freq;
        uniform float lacunarity;
        uniform float persistance;
        uniform int oct;
		uniform vec2 offset;
		uniform float scale;

		const int MAX_LAYER_COUNT = 32;
        uniform int LayerCount;
        uniform vec4 Layers[MAX_LAYER_COUNT];

        
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
		float noise_smooth(float t) {
			return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
			//return t;
		}
        
    float perlin2D(vec2 point, float freq)
	{
		point *= freq;
		int ix0 = int(floor(point.x));
		int iy0 = int(floor(point.y));
		float tx0 = point.x - ix0;
		float ty0 = point.y - iy0;
		float tx1 = tx0 - 1.f;
		float ty1 = ty0 - 1.f;
		ix0 &= HASH_MASK;
		iy0 &= HASH_MASK;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;

		int h0 = hash[ix0], h1 = hash[ix1];
		//define gradients
		vec2 gradients[8] = {
			vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1),
			normalize(vec2(1, 1)), normalize(vec2 (-1, 1)),
			normalize(vec2(1, -1)), normalize(vec2(-1, -1))
		};
		const int gradientMask = 7;
		vec2 g00 = gradients[hash[h0 + iy0] & gradientMask];
		vec2 g10 = gradients[hash[h1 + iy0] & gradientMask];
		vec2 g01 = gradients[hash[h0 + iy1] & gradientMask];
		vec2 g11 = gradients[hash[h1 + iy1] & gradientMask];

		float v00 = dot(g00, vec2( tx0, ty0 ));
		float v10 = dot(g10, vec2( tx1, ty0 ));
		float v01 = dot(g01, vec2( tx0, ty1 ));
		float v11 = dot(g11, vec2( tx1, ty1 ));

		tx0 = noise_smooth(tx0);
		ty0 = noise_smooth(ty0);

		return mix(mix(v00, v10, tx0), mix(v01, v11, tx0), ty0) * sqrt(2);
	}

		float sum(vec2 point, float freq, float lacunarity, float persistance, int oct) {
			float sum = perlin2D(point, freq);
			float amplitude = 1.f;
			float range = 1.f;
			for (int o = 1; o < oct; o++) {
				freq *= lacunarity;
				amplitude *= persistance;
				range += amplitude;
				sum += perlin2D(point, freq) * amplitude;
			}
			sum = sum / range;
			return sum;
		}
   

		vec3 pick_color(float h){
		    vec3 c = Layers[0].rgb;
			for(int i = 1; i < LayerCount; i++){
				if(h < Layers[i].a)
					break;
				c = Layers[i].rgb;
			}
			return c;
		}

		uniform mat4 VIEW;
		        
		void main()
		{

			float height = sum(world_position.xz + offset, freq, lacunarity, persistance, oct);
			height = (height + 1) / 2;

			//vec2 world_dx = dFdx(world_position.xz);
			//float hDx = sum(world_position.xz + world_dx + offset, freq, lacunarity, persistance, oct);
			//hDx = (hDx + 1) / 2;
			//vec2 world_dy = dFdy(world_position.xz);
			//float hDy = sum(world_position.xz + world_dy + offset, freq, lacunarity, persistance, oct);
			//hDy = (hDy + 1) / 2;
			//vec3 world_pos = vec3(world_position.x, height, world_position.z);
			//vec3 T = vec3(world_pos.x + world_dx.x, height + hDx, world_pos.z + world_dx.y) - world_pos;
			//vec3 B = vec3(world_pos.x + world_dy.x, height + hDy, world_pos.z + world_dy.y) - world_pos;

			// compute TBN
			vec3 T = dFdx(position);
			vec3 B = dFdy(position);
			vec3 N = cross(T, B);

            gPosition = position;
			mat3 TBN =  ((mat3(normalize(T), normalize(B), normalize(N))));
			vec3 normal_tex = normalize(texture2D(material.t_normals, uv).rgb * 2.0 - 1.0);
			gNormalShininess.rgb = normalize(TBN * normal_tex);
			//gNormalShininess.rgb = normalize(normal_tex);
			//gNormalShininess.rgb = normalize(N);
			gNormalShininess.a = material.shininess;
			gAlbedo.rgb = texture2D(material.t_diffuse, uv).rgb * color.rgb * pick_color(height);
			gAlbedo.a = length(texture2D(material.t_specular, uv).rgb);
			//gAlbedo.a = texture2D(material.t_specular, uv).a;
		}
        