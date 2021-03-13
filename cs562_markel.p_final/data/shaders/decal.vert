		#version 420

		/*
			CREDIT TO THIS FANTASTIC TUTORIAL
			https://martindevans.me/game-development/2015/02/27/Drawing-Stuff-On-Other-Stuff-With-Deferred-Screenspace-Decals/
		*/

		layout(location = 0) in vec3 _position;
				
		uniform mat4 MVP;
        
		void main()
		{
		gl_Position = MVP * vec4(_position, 1.f);
		}
        