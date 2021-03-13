		#version 420
        
    struct Material {
        sampler2D t_diffuse;
        sampler2D t_normals;
        sampler2D t_specular;
		vec3 diffuse;
		vec3 ambient;
		vec3 specular;
		vec3 emissive;
		float shininess;
	};
	uniform Material material;
    
    // LIGHT STUFF
    uniform vec3 LAmbient;
    const int MAX_LIGHT_COUNT = 32;
    
    struct DirectionalLight{
		vec3 diffuse;
		vec3 specular;
		vec3 direction;	// camera space!
	};
	uniform DirectionalLight LDirectional[MAX_LIGHT_COUNT];
	uniform int LDirectional_count;
    
    struct PointLight{
		vec3 diffuse;
		vec3 specular;
		vec3 position;	// camera space!
        vec3 attenuation;	// x: constant, y: linear, z: quadratic
	};
	uniform PointLight LPoint[MAX_LIGHT_COUNT];
	uniform int LPoint_count;
        
		uniform float light_intensity;
    
		in vec3 position;
		in vec3 normal;
		in vec2 uv;
        in vec4 color;
        in vec3 tangent;
        in vec3 bitangent;
		
		out vec4 FragColor;
        
        void compute_dir_light(DirectionalLight light, vec3 view, vec3 n, out vec3 diffuse, out vec3 specular){
            // assume already normalized
            
            //diffuse shading
            vec3 light_direction = light.direction;
            diffuse += light.diffuse * max(dot(n, light_direction), 0.0);
            //specular shading
            vec3 reflected = reflect(-light_direction, n);
            specular += light.specular * pow(max(dot(view, reflected), 0.0), material.shininess);
        }
        
        void compute_point_light(PointLight light, vec3 view, vec3 n, out vec3 diffuse, out vec3 specular){
            vec3 diff = light.position - position;
            float dist = length(diff);
            vec3 lightDir = diff / dist;
            //attenuation
            float att = 1.0 / (light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist);
            att = min(att, 1.0);
            
            //diffuse shading
            diffuse += light.diffuse * att * max(dot(n, lightDir), 0.0);
            
            //specular shading
            vec3 reflected = reflect(-lightDir, n);
            specular += light.specular * att * pow(max(dot(view, reflected), 0.0), material.shininess);
        }
        
        
        
		void main()
		{
		vec4 texColor = (texture2D(material.t_diffuse, uv) * color);
        
        vec3 diffuse = vec3(0,0,0);
        vec3 specular = vec3(0,0,0);
        
        vec3 view = normalize(-position);
        
        // transform to Tangent Space
        mat3 TBN =  ((mat3(normalize(tangent), normalize(bitangent), normalize(normal)))); 
        vec3 normal_tex = normalize(texture2D(material.t_normals, uv).rgb * 2.0 - 1.0);
		normal_tex = TBN * normal_tex;
		normal_tex = normalize(normal_tex);
        //vec3 normal_tex = normalize(normal);
		
        
        //light calculations
        for(int i = 0; i < LPoint_count; i++)
            compute_point_light(LPoint[i], view, normal_tex, diffuse, specular);
        for(int i = 0; i < LDirectional_count; i++)
            compute_dir_light(LDirectional[i], view, normal_tex, diffuse, specular);
        
       // diffuse *= material.diffuse;
       // specular *= material.specular * texture2D(material.t_specular, uv).r;
	   specular *= texture2D(material.t_specular, uv).r;
        
        //texColor *= vec4(LAmbient * material.ambient + diffuse + specular, 1);
        // do not apply SPECULAR and MATERIAL to match DEFERRED SHADING output
        //texColor *= vec4(LAmbient + diffuse, 1);
        
		FragColor = vec4( texColor.rgb * (light_intensity  * (diffuse + specular) + LAmbient), 1.0);

		}
        