		#version 430
        
        in vec3 position;
        in vec4 color;
        
        out vec4 FragColor;

        
		uniform float freq;
        uniform float lacunarity;
        uniform float persistance;
        uniform int oct;
		uniform vec3 offset;
        
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

	float smooth_fn(float t) {
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

		const vec3 gradients[16] = {
			normalize(vec3(1, 1, 0)),		normalize(vec3(-1, 1, 0	)), normalize(vec3( 1, -1, 0 )),
			normalize(vec3( -1, -1, 0)),	normalize(vec3( 1, 0, 1	)), normalize(vec3(-1,  0, 1 )),
			normalize(vec3( 1, 0, -1 )),	normalize(vec3(-1, 0,-1 )), normalize(vec3( 0,  1, 1 )),
			normalize(vec3( 0, -1, 1 )),	normalize(vec3( 0, 1,-1	)), normalize(vec3( 0, -1,-1 )),
			normalize(vec3( 1, 1, 0 )),		normalize(vec3( -1,1, 0	)), normalize(vec3( 0, -1, 1 )), normalize(vec3(0, -1, -1))
		};
		const int gradientMask = 15;


	float perlin3D(vec3 point, float freq)
	{
		point *= freq;
		int ix0 = int(floor(point.x));
		int iy0 = int(floor(point.y));
		int iz0 = int(floor(point.z));
		float tx0 = point.x - ix0;
		float ty0 = point.y - iy0;
		float tz0 = point.z - iz0;
		float tx1 = tx0 - 1.f;
		float ty1 = ty0 - 1.f;
		float tz1 = tz0 - 1.f;
		ix0 &= HASH_MASK;
		iy0 &= HASH_MASK;
		iz0 &= HASH_MASK;
		int ix1 = ix0 + 1;
		int iy1 = iy0 + 1;
		int iz1 = iz0 + 1;

		int h0 = hash[ix0];
		int h1 = hash[ix1];
		int h00 = hash[h0 + iy0];
		int h01 = hash[h0 + iy1];
		int h10 = hash[h1 + iy0];
		int h11 = hash[h1 + iy1];
		vec3 g000 = gradients[hash[h00 + iz0] & gradientMask];
		vec3 g001 = gradients[hash[h00 + iz1] & gradientMask];
		vec3 g010 = gradients[hash[h01 + iz0] & gradientMask];
		vec3 g011 = gradients[hash[h01 + iz1] & gradientMask];
		vec3 g100 = gradients[hash[h10 + iz0] & gradientMask];
		vec3 g101 = gradients[hash[h10 + iz1] & gradientMask];
		vec3 g110 = gradients[hash[h11 + iz0] & gradientMask];
		vec3 g111 = gradients[hash[h11 + iz1] & gradientMask];
		
		float v000 = dot(g000, vec3( tx0, ty0, tz0 ));
		float v001 = dot(g001, vec3( tx0, ty0, tz1 ));
		float v010 = dot(g010, vec3( tx0, ty1, tz0 ));
		float v011 = dot(g011, vec3( tx0, ty1, tz1 ));
		float v100 = dot(g100, vec3( tx1, ty0, tz0 ));
		float v101 = dot(g101, vec3( tx1, ty0, tz1 ));
		float v110 = dot(g110, vec3( tx1, ty1, tz0 ));
		float v111 = dot(g111, vec3( tx1, ty1, tz1 ));

		float tx = smooth_fn(tx0);
		float ty = smooth_fn(ty0);
		float tz = smooth_fn(tz0);

		return mix(mix(mix(v000, v100, tx), mix(v010, v110, tx), ty),
						mix(mix(v001, v101, tx), mix(v011, v111, tx), ty),
						tz);
	}

	float sum(vec3 point, float _freq, float _lacunarity, float _persistence, int _oct) {
			float sum = perlin3D(point, _freq);
			float amplitude = 1.f;
			float range = 1.f;
			for (int o = 1; o < _oct; o++) {
				_freq *= _lacunarity;
				amplitude *= _persistence;
				range += amplitude;
				sum += perlin3D(point, _freq) * amplitude;
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
        
		void main()
		{
        float noise_frag = sum(offset + normalize(position), freq, lacunarity, persistance, oct);
        noise_frag = (noise_frag + 1) / 2;
        
        //FragColor = vec4(vec3(noise_frag), 1);
        //return;
        
		FragColor = vec4(pick_color(noise_frag), 1);
        
		}