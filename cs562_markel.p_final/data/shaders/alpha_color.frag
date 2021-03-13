		#version 420
		uniform sampler2D diffuse_texture;
		
        in vec2 uv;
        
        out vec4 FragColor;
        
		void main()
		{
		vec4 texColor = texture2D(diffuse_texture, uv);
		FragColor = vec4(vec3(texColor.a), 1);
		}