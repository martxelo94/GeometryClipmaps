		#version 420
		layout(location = 0) in vec3 _position;
		layout(location = 1) in vec3 _normal;
		layout(location = 2) in vec2 _uv;
        layout(location = 3) in vec4 _color;
		layout(location = 4) in vec3 _tangent;
		layout(location = 5) in vec3 _bitangent;
		
        uniform mat4 PROJ;
        uniform mat4 VIEW;
		uniform mat4 WORLD;
		uniform mat4 MODEL;
        
        layout(location = 0) out vec3 position;
		layout(location = 1) out vec3 normal;
		layout(location = 2) out vec2 uv;
        layout(location = 3) out vec4 color;
		layout(location = 4) out vec3 tangent;
		layout(location = 5) out vec3 bitangent;
        
		void main()
		{
        color = _color;
		uv = _uv;
		mat4 MV = VIEW * WORLD * MODEL;
		mat4 MVP = PROJ * MV;
		normal = transpose(inverse(mat3(MV))) * _normal;
		tangent = mat3(MV) * _tangent;
		bitangent = mat3(MV) * _bitangent;
		position = vec3(MV * vec4(_position, 1.f));
		gl_Position = MVP * vec4(_position, 1.f);
		}
        