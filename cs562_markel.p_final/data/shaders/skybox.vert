		#version 420
		layout(location = 0) in vec3 _position;
        layout(location = 3) in vec4 _color;
        
		uniform mat4 M;
        out vec4 color;
		out vec3 TexCoords;
		void main()
		{
        color = _color;
        
        #if 0
        color = vec4((_position), 1);
        
        
        #endif
        
		TexCoords = _position;
        
        
		gl_Position = (M * vec4(_position, 1.f)).xyww;
		}
        