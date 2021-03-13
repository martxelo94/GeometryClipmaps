		#version 420

        layout(location = 3) in vec4 color;
        out vec4 FragColor;
        
		void main()
		{
            FragColor = color;
		}