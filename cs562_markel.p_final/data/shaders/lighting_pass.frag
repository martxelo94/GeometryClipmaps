		#version 420
        
        out vec4 FragColor;
        
		in vec2 uv;
		
        uniform sampler2D gPosition;
        uniform sampler2D gNormalShininess;
        uniform sampler2D gAlbedoSpec;
		uniform sampler2D t_ambient_occlusion;
		
		const int MAX_LIGHT_COUNT = 1;
		struct PointLight{
				vec3 diffuse;
				vec3 specular;
				vec3 position;	// camera space!
				vec3 attenuation;	// x: constant, y: linear, z: quadratic
		};
		uniform PointLight LPoint[MAX_LIGHT_COUNT];
		uniform int LPoint_count;
			
	    struct DirectionalLight{
			vec3 diffuse;
			vec3 specular;
			vec3 direction;	// camera space!
		};
		uniform DirectionalLight LDirectional[1];
		uniform int LDirectional_count;


		uniform vec3 LAmbient;
        uniform float light_intensity;

		void compute_dir_light(DirectionalLight light, vec3 view, vec3 n, float shininess, out vec3 diffuse, out vec3 specular);
		void compute_point_light(PointLight light, vec3 view, vec3 n, vec3 p, float shininess, out vec3 diffuse, out vec3 specular);
        
		void main()
		{
			vec3 FragPos = texture2D(gPosition, uv).xyz;
			vec3 Normal = texture2D(gNormalShininess, uv).xyz;
			float Shininess = texture2D(gNormalShininess, uv).a;
			vec3 Albedo = texture2D(gAlbedoSpec, uv).rgb;
			float Specular = texture2D(gAlbedoSpec, uv).a;
			
			
			// calculate lighting
			vec3 diffuse;
			vec3 specular;
			vec3 view = normalize(-FragPos);
			
			// single light pass
			for(int i = 0; i < LPoint_count; i++){
				compute_point_light(LPoint[i], view, Normal, FragPos, Shininess, diffuse, specular);
			}
			for(int i = 0; i < LDirectional_count; i++){
				compute_dir_light(LDirectional[i], view, Normal, Shininess, diffuse, specular);
			}

			// output light intensity only!!!
			FragColor = vec4( Albedo * (light_intensity * (diffuse + specular * Specular) + LAmbient * texture2D(t_ambient_occlusion, uv).r), 1.0);
		}
        

		void compute_dir_light(DirectionalLight light, vec3 view, vec3 n, float shininess, out vec3 diffuse, out vec3 specular){
            // assume already normalized
            
            //diffuse shading
            vec3 light_direction = light.direction;
            diffuse += light.diffuse * max(dot(n, light_direction), 0.0);
            //specular shading
            vec3 reflected = reflect(-light_direction, n);
            specular += light.specular * pow(max(dot(view, reflected), 0.0), shininess);
        }
		void compute_point_light(PointLight light, vec3 view, vec3 n, vec3 p, float shininess, out vec3 diffuse, out vec3 specular){
            vec3 diff = light.position - p;
            float dist = length(diff);
            vec3 lightDir = diff / dist;
            //attenuation
            float att = 1.0 / (light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist);
            att = min(att, 1.0);
            
            //diffuse shading
            diffuse += light.diffuse * att * max(dot(n, lightDir), 0.0);
            
            //specular shading
            vec3 reflected = reflect(lightDir, n);
            specular += light.specular * att * pow(max(dot(view, reflected), 0.0), shininess);
        }
