#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D depth_texture;
uniform float cam_far;
uniform float cam_near;


const mat3 sx = mat3( 
    1.0, 2.0, 1.0, 
    0.0, 0.0, 0.0, 
   -1.0, -2.0, -1.0 
);
const mat3 sy = mat3( 
    1.0, 0.0, -1.0, 
    2.0, 0.0, -2.0, 
    1.0, 0.0, -1.0 
);

float linearize_depth(float z)
{
          z = z * 2.f - 1.f;  // back to ndc
	return (2.0 * cam_near) / (cam_far + cam_near - z * (cam_far - cam_near));
}

void main()
{
	float depth = texture2D(depth_texture, uv).r;
    depth = linearize_depth(depth);
	mat3 I;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
            float d = texelFetch(depth_texture, ivec2(gl_FragCoord) + ivec2(i - 1, j - 1), 0).r;
			I[i][j] = linearize_depth(d);
		}
	}
	float gx = dot(sx[0], I[0]) + dot(sx[1], I[1]) + dot(sx[2], I[2]);
	float gy = dot(sy[0], I[0]) + dot(sy[1], I[1]) + dot(sy[2], I[2]);
	
	float g = sqrt(gx * gx + gy * gy);
	g = depth - g;
	FragColor = vec4(vec3(depth - g), 1.0);
}