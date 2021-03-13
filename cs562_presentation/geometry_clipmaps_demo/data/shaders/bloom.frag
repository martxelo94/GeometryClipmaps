#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D color_texture;
uniform sampler2D blur_texture;

void main()
{
	vec3 color = texture2D(color_texture, uv).rgb;
	vec3 blur = texture2D(blur_texture, uv).rgb;
	FragColor = vec4((color + blur) / 2, 1);
    
}