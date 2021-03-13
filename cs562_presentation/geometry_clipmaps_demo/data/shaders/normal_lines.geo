#version 420

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout(location = 0) in vec3 position[];
layout(location = 1) in vec3 normal[];

layout(location = 3) out vec4 color;


const float LINE_SIZE = 10.0f;

void main(){
    gl_Position = vec4(position[0], 1);
    color = vec4(1, 1, 1, 1);
    EmitVertex();
    gl_Position = vec4(position[0] + normal[0] * LINE_SIZE, 1.0);
    color = vec4(1, 1, 1, 1);
    EmitVertex();
    
    EndPrimitive();
}
