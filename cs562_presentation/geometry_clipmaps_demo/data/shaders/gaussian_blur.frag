#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D color_texture;
uniform float deviation;
uniform int radius;

#define PI 3.1415926535897932384626433832795

subroutine vec4 BlurPass();
subroutine uniform BlurPass blurFunction;

subroutine(BlurPass)
vec4 HorizontalPass(){
	vec4 color;

	float deviation2 = deviation * deviation;
	float constant = 1.0 / sqrt(2 * PI * deviation2);
	float divisor = 2 * deviation2;

	for(int i = -radius; i <= radius; i++){
		
		float kernel = constant * exp(- i * i / divisor);

		vec4 pixColor = texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(i, 0), 0);

		color += pixColor * kernel;
	}
	return color;
}

subroutine(BlurPass)
vec4 VerticalPass(){
	vec4 color;

	float deviation2 = deviation * deviation;
	float constant = 1.0 / sqrt(2 * PI * deviation2);
	float divisor = 2 * deviation2;

	for(int i = -radius; i <= radius; i++){
		
		float kernel = constant * exp(- i * i / divisor);

		vec4 pixColor = texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(0, i), 0);

		color += pixColor * kernel;
	}
	return color;
}
void main()
{
	FragColor = blurFunction();

}