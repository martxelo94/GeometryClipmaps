		#version 420
		
        layout(location = 2) in vec2 _uv;
        layout(location = 3) in vec4 _color;

        
        out vec4 FragColor;
        
        uniform sampler2D diffuse_texture;
        uniform vec4 color;
        
		void main()
		{
		vec4 texColor = (texture2D(diffuse_texture, _uv) * color) * _color;
		if(texColor.a < 0.1) discard;
		FragColor = texColor;
		}