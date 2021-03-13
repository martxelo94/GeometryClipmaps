#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D color_texture;
uniform int radius;

void main()
{
	vec3 color;
	for(int i = -radius; i <= radius; i++){
		for(int j = -radius; j <= radius; j++){
			color += texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(i, j), 0).rgb;
		}
	}
	float R =  2 * radius + 1;
	FragColor = vec4(color / (R * R), 1);
}