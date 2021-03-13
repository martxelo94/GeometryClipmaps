		#version 420
		layout(location = 0) in vec3 _position;
		layout(location = 3) in vec4 _color;
		out vec4 color;
        
        uniform mat4 M;
        
		void main()
		{
		color = _color;
        gl_Position = M * vec4(_position, 1.0f);
		}
        