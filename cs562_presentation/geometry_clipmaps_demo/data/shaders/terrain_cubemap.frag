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
        