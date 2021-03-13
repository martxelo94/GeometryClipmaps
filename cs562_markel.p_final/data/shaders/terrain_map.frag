		#version 430
        
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
		uniform float scale;
		uniform float height_scale;
		uniform int mipmap_level;
		uniform int use_color_mipmaps;	// DEBUG COLOR
		uniform sampler2D t_height;
		uniform sampler2D t_noise;

		uniform mat4 VIEW;

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

		vec3 pick_mipmap_color(int lvl){
			vec3 color = vec3(1);
			
			if(lvl < LayerCount)
				return Layers[lvl].rgb;
			return color;
		}
		
		vec3 world_sample(sampler2D tex, vec2 world_point)
		{

			return textureLod(tex, vec2(world_point.x, world_point.y) / textureSize(tex, 0) * 1.0 / scale + vec2(0.5, 0.5), mipmap_level).rgb;
		}




		void main()
		{

			//float height = texture2D(t_height, world_position.xz / textureSize(t_height, 0) * freq).r;
			float height = world_sample(t_height, world_position.xz).r;
			float noise = world_sample(t_noise, world_position.xz).r;
			//noise = (noise + 1) / 2;
			height += noise / 10;
			height = clamp(height, 0, 1);
			height *= height_scale;

		#if 0
			//mat4 invVIEW = inverse(VIEW);
			//vec2 world_dx = dFdx();
			vec2 world_dx = dFdx(world_position.xz);
			//float hDx = texture2D(t_height, (world_position.xz + world_dx) / textureSize(t_height, 0) * freq).r;
			float hDx = world_sample(t_height, world_position.xz + world_dx).r;
			hDx = (hDx + 1) / 2;
			hDx *= scale;
			vec2 world_dy = dFdy(world_position.xz);
			//float hDy = texture2D(t_height, (world_position.xz + world_dy) / textureSize(t_height, 0) * freq).r;
			float hDy = world_sample(t_height, world_position.xz + world_dy).r;
			hDy = (hDy + 1) / 2;
			hDy *= scale;
			vec3 world_pos = vec3(world_position.x, height, world_position.z);
			vec3 T = mat3(VIEW) * vec3(world_dx.x, hDx, world_dx.y);
			vec3 B = mat3(VIEW) * vec3(world_dy.x, hDy, world_dy.y);
			vec3 N = cross(T, B);
		#else
			// compute TBN
			vec3 T = dFdx(position);
			vec3 B = dFdy(position);
			vec3 N = cross(T, B);
		#endif

            gPosition = position;
			mat3 TBN =  ((mat3(normalize(T), normalize(B), normalize(N))));
			vec3 normal_tex = normalize(texture2D(material.t_normals, uv).rgb * 2.0 - 1.0);
			//gNormalShininess.rgb = normalize(TBN * normal_tex);
			gNormalShininess.rgb = normalize(N);
			//gNormalShininess.rgb = normalize(normal);
			gNormalShininess.a = material.shininess;
	

			gAlbedo.rgb =	world_sample(material.t_diffuse, world_position.xz)
							//texture2D(material.t_diffuse, uv).rgb 
							* color.rgb	
							//* pick_color(height / height_scale) 
							;

			// DEBUGGING
			if(use_color_mipmaps == 1){
				if(mod(mipmap_level, 2) == 0)
					gAlbedo.rgb += vec3(0.5);
				else
					gAlbedo.rgb /= 2;
			}



			gAlbedo.a = length(world_sample(material.t_specular, world_position.xz).rgb);
			//gAlbedo.a = texture2D(material.t_specular, uv).a;
		}
        