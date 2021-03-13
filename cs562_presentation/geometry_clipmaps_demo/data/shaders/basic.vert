		#version 420
		layout(location = 0) in vec3 _position;
		layout(location = 1) in vec3 _normal;
		layout(location = 2) in vec2 _uv;
        layout(location = 3) in vec4 _color;
        layout(location = 4) in vec3 _tangent;
        layout(location = 5) in vec3 _bitangent;

        layout(location = 2) out vec2 uv;
        layout(location = 3) out vec4 color;
        

        uniform mat4 M;
        

		void main()
		{
        color = _color;
		uv = _uv;
		gl_Position = M * vec4(_position, 1.f);
		}
        