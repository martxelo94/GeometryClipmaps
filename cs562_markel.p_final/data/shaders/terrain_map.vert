		#version 430

		layout(location = 0) in vec3 _position;
		layout(location = 1) in vec3 _normal;
		layout(location = 2) in vec2 _uv;
        layout(location = 3) in vec4 _color;
		layout(location = 4) in vec3 _tangent;
		layout(location = 5) in vec3 _bitangent;
		
		layout(location = 0) out vec3 position;
		layout(location = 1) out vec3 normal;
		layout(location = 2) out vec2 uv;
        layout(location = 3) out vec4 color;
		layout(location = 4) out vec3 tangent;
		layout(location = 5) out vec3 bitangent;

		layout(location = 6) out vec3 world_position;

        uniform mat4 PROJ;
        uniform mat4 VIEW;
		uniform mat4 WORLD;
        
        
		uniform float freq;
		uniform float scale;
		uniform float height_scale;
		uniform int mipmap_level;
		uniform sampler2D t_height;
		uniform sampler2D t_noise;

		uniform int ring_case;
		const int base_mipmap = 0;

		uniform float snap_precision;


		const int MAX_LAYER_COUNT = 32;
        uniform int LayerCount;
        uniform vec4 Layers[MAX_LAYER_COUNT];


		vec3 pick_color(float h){
			
		    vec4 c = Layers[0];
			vec4 prev_c = c;
			for(int i = 1; i < LayerCount; i++){
				if(h < Layers[i].a)
					break;
				prev_c = c;
				c = Layers[i];
			}
			return c.rgb;
			if(c.a == prev_c.a)
				return c.rgb;
			float t = (h - prev_c.a) / (c.a - prev_c.a);
			return prev_c.rgb * (1 - t) + c.rgb * (t);
		}

		float world_sample(sampler2D tex, vec2 world_point)
		{
			if(ring_case == 1){
				//HARDCODED!
				float SIZE = 2;
				if(abs(_position.x) >= SIZE || abs(_position.y) >= SIZE || abs(_position.z) >= SIZE)
					return textureLod(tex, world_point / textureSize(tex, 0) * 1.0 / scale + vec2(0.5, 0.5), max(0, (mipmap_level + 1) + base_mipmap)).r;
			}
			return textureLod(tex, world_point / textureSize(tex, 0) * 1.0 / scale + vec2(0.5, 0.5), max(0, mipmap_level + base_mipmap)).r;
		}


		void main()
		{

		// compute height
		vec4 worldPos = WORLD * vec4(_position.x, 0, _position.z, 1);
		worldPos.x = -worldPos.x;
		worldPos.z = worldPos.z;

		float height = world_sample(t_height, worldPos.xz);
		//height = (height + 1) / 2;	// for noise functions

		float delta = scale * 1.0 / textureSize(t_height, 0).x;
		vec3 T = vec3(delta, world_sample(t_height, vec2(worldPos.x + delta, worldPos.z) - height), 0);
		vec3 B = vec3(0, world_sample(t_height, vec2(worldPos.x, worldPos.z + delta) - height), delta);
		vec3 N = -cross(T, B);
		N = normalize(N);


		float noise = world_sample(t_noise, worldPos.xz);
		//noise = (noise - 0.5) * 2;	// make range from [0,1] to [-1,1]
		height += noise / 10;
		height = clamp(height, 0, 1);
        color = _color * vec4(pick_color(height), 1);
		


		height *= height_scale;

		// lower precision
		//float precision_level = pow(2, mipmap_level);
		//height = floor(height * precision_level ) / precision_level ;

		world_position = vec3(worldPos.x, height, worldPos.z);


		uv = _uv;
		mat4 WORLD_no_y = WORLD;
		WORLD_no_y[1][1] = 1.f;
		mat4 MV = VIEW * WORLD_no_y;
		mat4 MVP = PROJ * MV;
		//normal = transpose(inverse(mat3(MV))) * _normal;
		normal = transpose(inverse(mat3(MV))) * N;

		tangent = mat3(MV) * _tangent;
		bitangent = mat3(MV) * _bitangent;
		position = vec3(MV * vec4(_position.x, height, _position.z, 1.f));
		gl_Position = MVP * vec4(_position.x, height, _position.z, 1.f);
		}
        