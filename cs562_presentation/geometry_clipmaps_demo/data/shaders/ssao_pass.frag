		#version 420
        
        out vec4 FragColor;
        
		in vec2 uv;
		
        uniform sampler2D t_position;
		uniform sampler2D t_normal;
		uniform sampler2D t_noise;
		
		uniform float radius;
		uniform float attenuation;
		uniform float scale;
		uniform float angle_bias;

		uniform mat4 PROJ;	// need to project and sample random points
        

		vec3 get_view_position(vec2 texCoord)
		{
			// position from gBuffer in View Space
			float sampledDepth = texture2D(t_position, texCoord).r;
			// clip space
			vec4 sampledPosition = vec4(texCoord * 2.0 - 1.0, sampledDepth * 2.0 - 1.0, 1.0);
			
			// to view space
			sampledPosition = inverse(PROJ) * sampledPosition;
			sampledPosition /= sampledPosition.w;
			
			return sampledPosition.xyz;
		}
		vec3 get_view_position(vec2 texCoord, float sampledDepth)
		{
			// clip space
			vec4 sampledPosition = vec4(texCoord * 2.0 - 1.0, sampledDepth * 2.0 - 1.0, 1.0);
			
			// to view space
			sampledPosition = inverse(PROJ) * sampledPosition;
			sampledPosition /= sampledPosition.w;
			
			return sampledPosition.xyz;
		}
		void main()
		{
			//vec3 fragPosition = texture2D(t_position, uv).xyz;
			float depth = texture2D(t_position, uv).r;
			vec3 fragPosition = get_view_position(uv, depth);
			

			vec3 tangent = dFdx(fragPosition);
			vec3 bitangent = dFdy(fragPosition);
			vec3 fragNormal = normalize(cross(tangent, bitangent));
			//vec3 fragNormal = texture2D(t_normal, uv).xyz;

			mat3 TBN = mat3(normalize(tangent), normalize(bitangent), fragNormal);


			float R = radius / depth;
			float bias = angle_bias / depth;

			ivec2 noiseSize = textureSize(t_noise, 0);

			float block_factor = 0.0;

			for(int x = 0; x < noiseSize.x; ++x){
				for(int y = 0; y < noiseSize.y; ++y){
					vec2 noiseTexCoord = vec2(float(x) / noiseSize.x, float(y) / noiseSize.y);
					vec4 rand_lookup = texture2D(t_noise, noiseTexCoord);
					vec4 rotated_vector = mat4(TBN) * vec4(normalize(rand_lookup.xy), 0, 1);
					vec4 offset = rotated_vector * R;
					vec3 samplePos = fragPosition + offset.xyz;
					vec4 projectedPoint = PROJ * vec4(samplePos, 1);
					vec2 screenPoint = projectedPoint.xy / projectedPoint.w;
					screenPoint = (screenPoint.xy * 0.5) + vec2(0.5, 0.5);
					//vec3 neighbor_lookup = get_view_position(screenPoint);
					//neighbor_lookup.z = -1.0 / neighbor_lookup.z;
					//float z_diff = max(0, samplePos.z - neighbor_lookup.z);

					float neighbor_depth = texture2D(t_position, screenPoint).r;
					float z_diff = depth - neighbor_depth;

					if(z_diff + bias < R && z_diff < 0)
						block_factor += 1 * attenuation;
				}
			}
			block_factor /= noiseSize.x * noiseSize.y;
			//block_factor = 1 - block_factor;
			block_factor *= scale;

			FragColor = vec4(block_factor, block_factor, block_factor, 1);
		}
        