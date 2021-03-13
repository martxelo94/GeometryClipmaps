#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D color_texture;
uniform sampler2D edges_texture;

void main()
{
	float edge = texture2D(edges_texture, uv).r;
	if(edge > 0){
		vec3 color;
		for(int i = -1; i <= 1; i++){
			for(int j = -1; j <= 1; j++){
				color += texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(i, j), 0).rgb;
			}
		}
		color /= 9;
		FragColor = vec4(color, 1);
	}else{
		FragColor = texture2D(color_texture, uv);
	}
}