		#version 420
		uniform samplerCube skybox;
		in vec3 TexCoords;
        in vec4 color;
		void main()
		{
		vec4 texColor = (texture(skybox, TexCoords) * color);
        gl_FragColor = texColor;
        
        #if 0
        float G = max(TexCoords.x, max(TexCoords.y, TexCoords.z));
        if(G == TexCoords.x)
            gl_FragColor = vec4(1, 0, 0, 1);
        if(G == TexCoords.y)
            gl_FragColor = vec4(0, 1, 0, 1);
        if(G == TexCoords.z)
            gl_FragColor = vec4(0, 0, 1, 1);
        
        #endif
        
		}