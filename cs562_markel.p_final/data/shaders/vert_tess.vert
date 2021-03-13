		#version 420
		layout(location = 0) in vec3 _position;
		layout(location = 1) in vec3 _normal;
		layout(location = 2) in vec2 _uv;
        //layout(location = 3) in vec4 _color;
		//layout(location = 4) in vec3 _tangent;
		//layout(location = 5) in vec3 _bitangent;
		        
        layout(location = 0) out vec3 position;
		layout(location = 1) out vec3 normal;
		layout(location = 2) out vec2 uv;
        //layout(location = 3) out vec4 color;
		//layout(location = 4) out vec3 tangent;
		//layout(location = 5) out vec3 bitangent;

		uniform mat4 MVP;
        
		void main()
		{
        //color = _color;
		//tangent = _tangent;
		//bitangent = _bitangent;
		uv = _uv;
		normal = _normal;
		position = _position;
		gl_Position = vec4(_position, 1.f);
		}
        