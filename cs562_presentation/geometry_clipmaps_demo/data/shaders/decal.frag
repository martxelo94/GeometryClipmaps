		#version 420

		/*
			CREDIT TO THIS FANTASTIC TUTORIAL
			https://martindevans.me/game-development/2015/02/27/Drawing-Stuff-On-Other-Stuff-With-Deferred-Screenspace-Decals/
		*/
        
        layout(location = 0) out vec4 gDiffuse;	// DO NOT CHANGE ALPHA!!!
		layout(location = 1) out vec4 gNormal;	// DO NOT CHANGE ALPHA!!!

        		
		uniform sampler2D t_diffuse;
		uniform sampler2D t_normal;
		uniform sampler2D t_depth;

		uniform float cos_angle_limit;
		uniform int render_mode;
		uniform int texture_w;
		uniform int texture_h;

		uniform mat4 INV_PROJ;
		uniform mat4 INV_MV;
		uniform mat4 MV;
		

		vec3 get_view_position(vec2 texCoord)
		{
			// position from gBuffer in View Space
			float sampledDepth = texture2D(t_depth, texCoord).r;
			// clip space
			vec4 sampledPosition = vec4(texCoord * 2.0 - 1.0, sampledDepth * 2.0 - 1.0, 1.0);
			
			// to view space
			sampledPosition = INV_PROJ * sampledPosition;
			sampledPosition /= sampledPosition.w;
			
			return sampledPosition.xyz;
		}
		vec3 get_model_position(vec2 texCoord)
		{
			// view space
			vec4 sampledPosition = vec4(get_view_position(texCoord), 1);
			
			// object space
			sampledPosition = INV_MV * sampledPosition;
			return sampledPosition.xyz;
		}
        
		void main()
		{

			// get uvs from screen position
			vec2 texCoord = vec2(gl_FragCoord.x / texture_w, gl_FragCoord.y / texture_h);
			
			if(render_mode == 2){
				float sampledDepth = texture2D(t_depth, texCoord).r;
				if(sampledDepth < gl_FragCoord.z)
					discard;
				gDiffuse.rgb = vec3(1,1,1);
				return;
			}

			vec3 sampledPosition = get_model_position(texCoord);

			float x_bound = 0.5 - abs(sampledPosition.x);
			float y_bound = 0.5 - abs(sampledPosition.y);
			float z_bound = 0.5 - abs(sampledPosition.z);

			// perform bounds check on XY axis
			if(x_bound < 0 || y_bound < 0 || z_bound < 0)
				discard;

			if(render_mode == 1){
				gDiffuse.rgb = vec3(1,1,1);
				return;
			}

			// discard transparent pixels
			vec4 color =  texture2D(t_diffuse, sampledPosition.xy + 0.5);
			if(color.a < 0.1f)
				discard;
			
			// compute normals
			//vec3 ddx = get_model_position(texCoord + vec2(1.0 / texture_w, 0)) - sampledPosition;
			//vec3 ddy = get_model_position(texCoord + vec2(0, 1.0 / texture_h)) - sampledPosition;
			vec3 viewPosition = get_view_position(texCoord);
			vec3 ddx = get_view_position((gl_FragCoord.xy + vec2(1,0)) / vec2(texture_w, texture_h)) - viewPosition;
			vec3 ddy = get_view_position((gl_FragCoord.xy + vec2(0,1)) / vec2(texture_w, texture_h)) - viewPosition;
			vec3 normal = normalize(cross(ddx, ddy));

			// angle discard
			if(abs(dot(mat3(INV_MV) * normal, vec3(0, 0, 1))) < cos_angle_limit)
				discard;

			vec3 tangent = normalize(ddx);
			vec3 bitangent = normalize(ddy);

			mat3 NormalMtx = mat3(MV);
			mat3 TBN = mat3(tangent, bitangent, normal);
			normal = texture2D(t_normal, sampledPosition.xy + 0.5).rgb;

			// draw buffers
			gDiffuse.rgb = color.rgb;
			gNormal.xyz = TBN * normal;
		}
        