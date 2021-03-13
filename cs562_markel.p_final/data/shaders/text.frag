		#version 420
		uniform sampler2D tex;
		uniform vec4 color;
		
        in vec2 uv;
        
        out vec4 FragColor;
        
		void main()
		{
		vec4 texColor = texture2D(tex, uv);
		//if(texColor.a < 0.1) discard;
		//if(texColor.r < 0.1){
		//	FragColor = vec4(0, 1, 0, 1);
		//	return;
		//}
		FragColor = texColor * color;
		}