		#version 420
        
        out vec4 FragColor;
        
        in vec2 uv;
        
        uniform sampler2D shadowMap;

		void main()
		{
            float depthValue = texture(shadowMap, uv).r;
            FragColor = vec4(vec3(depthValue), 1.0);
            //FragColor = texture(shadowMap, uv);
		}