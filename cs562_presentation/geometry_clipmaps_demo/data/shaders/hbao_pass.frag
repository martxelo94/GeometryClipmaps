		#version 420
        
        out vec4 FragColor;
        
		in vec2 uv;
		
        uniform sampler2D t_position;
		uniform sampler2D t_normal;
		uniform int t_width;
		uniform int t_height;

		uniform sampler2D t_noise;
		
		uniform float radius;
		uniform float angle_bias;
		uniform float attenuation;
		uniform float scale;
		uniform int direction_count;
		uniform int step_count;

		uniform mat4 PROJ;	// need to project and sample random points
        

		#define PI 3.1415926535897932384626433832795
		#define PI2 2.0 * PI

		/*
			Credit to Neil Mendoza for this function
			http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
		*/
		mat4 rotationMatrix(vec3 axis, float angle)
		{
		    axis = normalize(axis);
		    float s = sin(angle);
		    float c = cos(angle);
		    float oc = 1.0 - c;
		    
		    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
		                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
		                0.0,                                0.0,                                0.0,                                1.0);
		}

		mat2 rotationMatrix(float angle)
		{
			float c = cos(angle);
			float s = sin(angle);
			return mat2(c, -s, s, c);
		}

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
			vec3 fragPosition = get_view_position(uv);
			//float fragDepth = texture2D(t_position, uv).r;
			//vec3 fragPosition = get_view_position(uv, fragDepth);
			vec3 fragNormal = texture2D(t_normal, uv).xyz;

			vec3 fragTangent = dFdx(fragPosition);
			vec3 fragBitangent = dFdy(fragPosition);
			fragNormal = normalize(cross(fragTangent, fragBitangent));
			mat3 TBN = mat3(normalize(fragTangent), normalize(fragBitangent), fragNormal);


			float R = radius / -fragPosition.z;
			//float R = radius / fragDepth;


			//if(R > 1){
			//	FragColor = vec4(1, 0, 0, 1);
			//	return;
			//}

			float WAO = 0;

			float angle_step = PI2 / (direction_count + 1);
			float angle = 0;

			for(int i = 0 ; i < direction_count; i++){
				angle += angle_step;

				angle = texture2D(t_noise, vec2(float(i) / direction_count, 0)).r * PI2;

				vec2 dir = vec2(cos(angle), sin(angle));
				dir *= R;
				dir /= vec2(t_width, t_height);


				//get surface tangent
				vec3 proyectedTangent = vec3(dir, 0) - dot(vec3(dir, 0) - fragPosition, fragNormal) * fragNormal;

				float t_angle = atan(proyectedTangent.z / length(proyectedTangent.xy));
				float max_elevation_angle = 0;
				vec3 horizon_point = vec3(0);

				for(int j = 0; j < step_count; j++){
				
					vec3 direction = TBN * vec3(dir * (float(j + 1) / step_count), 0);	
					//vec2 screenPoint = uv + dir * (float(j + 1) / step_count);
					vec2 screenPoint = uv + direction.xy;

					// snap to texel coord
					screenPoint = vec2(floor(screenPoint.x * t_width), floor(screenPoint.y * t_height)) / vec2(t_width, t_height);
					
					// horizon point
					//vec3 neighbor_lookup = texture2D(t_position, screenPoint).xyz;
					vec3 neighbor_lookup = get_view_position(screenPoint);
					vec3 D = neighbor_lookup - fragPosition;

					if(length(D) > R)
						continue;

					// horizon angle
					float elevation_angle = atan(D.z / length(D.xy));

					if(elevation_angle < t_angle + angle_bias)
						continue;

					//elevation_angle = max(t_angle, elevation_angle);

					if(max_elevation_angle < elevation_angle)
					{
						max_elevation_angle = elevation_angle;
						horizon_point = D;
					}

				}
				float r = length(horizon_point) / R;
				float W = max(0, 1 - r * r);
				float AO = sin(max_elevation_angle) - sin(t_angle);
				//AO /= direction_count;
				WAO += attenuation * AO * W;
			}
			WAO /= direction_count;

			float A = WAO / PI2;
			A *= scale;

			FragColor = vec4(A, A, A, 1);
		}
        