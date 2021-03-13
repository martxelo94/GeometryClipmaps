		#version 420
        
        out vec4 FragColor;
        
        in vec2 uv;
        
        uniform sampler2D shadowMap;
        uniform float cam_far;
		uniform float cam_near;
		
		float linearize_depth(float z)
		{
            z = z * 2.f - 1.f;  // back to ndc
			return (2.0 * cam_near) / (cam_far + cam_near - z * (cam_far - cam_near));
		}
		
		void main()
		{
            float depthValue = texture(shadowMap, uv).r;
			depthValue = linearize_depth(depthValue);
            FragColor = vec4(vec3(depthValue), 1.0);
            //FragColor = texture(shadowMap, uv);
		}