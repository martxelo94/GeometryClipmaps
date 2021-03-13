#version 420
in vec2 uv;
out vec4 FragColor;
uniform sampler2D color_texture;
uniform float range_deviation;
uniform float spatial_deviation;
uniform int radius;

#define PI 3.1415926535897932384626433832795

subroutine vec4 BlurPass();
subroutine uniform BlurPass blurFunction;

subroutine(BlurPass)
vec4 HorizontalPass(){
	vec4 color;
	float W = 0;
	vec4 original_color = texture2D(color_texture, uv);

	float range_deviation2 = range_deviation * range_deviation;
	float spatial_deviation2 = spatial_deviation * spatial_deviation;

	for(int i = -radius; i <= radius; i++){
		
		vec4 pixColor = texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(i, 0), 0);
		vec3 diff = pixColor.rgb - original_color.rgb;
		float kernel = exp(-(i * i / (2 * range_deviation2)) - (dot(diff, diff) / (2 * spatial_deviation2)));
		color += pixColor * kernel;
		W += kernel;
	}
	return color / W;
}

subroutine(BlurPass)
vec4 VerticalPass(){
	vec4 color;
	float W = 0;
	vec4 original_color = texture2D(color_texture, uv);

	float range_deviation2 = range_deviation * range_deviation;
	float spatial_deviation2 = spatial_deviation * spatial_deviation;

	for(int i = -radius; i <= radius; i++){
		
		vec4 pixColor = texelFetch(color_texture, ivec2(gl_FragCoord) + ivec2(0, i), 0);
		vec3 diff = pixColor.rgb - original_color.rgb;
		float kernel = exp(-(i * i / (2 * range_deviation2)) - (dot(diff, diff) / (2 * spatial_deviation2)));
		color += pixColor * kernel;
		W += kernel;
	}
	return color / W;
}
void main()
{
	FragColor = blurFunction();

}