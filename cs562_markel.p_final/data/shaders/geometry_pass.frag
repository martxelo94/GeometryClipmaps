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
        
    struct Material {
        sampler2D t_diffuse;
		sampler2D t_normals;
        sampler2D t_specular;
		float shininess;
	};
	uniform Material material;     
		        
		void main()
		{
            gPosition = position;
			mat3 TBN =  ((mat3(normalize(tangent), normalize(bitangent), normalize(normal))));
			vec3 normal_tex = normalize(texture2D(material.t_normals, uv).rgb * 2.0 - 1.0);
			gNormalShininess.rgb = TBN * normal_tex;
			gNormalShininess.a = material.shininess;
			gAlbedo.rgb = texture2D(material.t_diffuse, uv).rgb * color.rgb;
			gAlbedo.a = length(texture2D(material.t_specular, uv).rgb);
			//gAlbedo.a = texture2D(material.t_specular, uv).a;
		}
        