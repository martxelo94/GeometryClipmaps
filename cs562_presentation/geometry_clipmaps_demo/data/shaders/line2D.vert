		#version 420
		layout(location = 0) in vec2 _position;
		layout(location = 1) in vec4 _color;
		out vec4 color;
		void main()
		{
		color = _color;
        gl_Position = vec4(_position, 0.0f, 1.0f);
		}
        