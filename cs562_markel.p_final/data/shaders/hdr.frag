#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D hdr_texture;
uniform int radius;

const float threshold = 1.1f;

void main()
{
	vec3 color = texture2D(hdr_texture, uv).rgb;
    // quick discard
    float len2 = dot(color, color);
    if(len2 < threshold * threshold)
        discard;
    FragColor = vec4(color, 1);
    
}