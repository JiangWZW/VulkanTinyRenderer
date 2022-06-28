#version 460

layout(location = 0) out vec3 fragColor;

vec3 vertCoords[3] = vec3[](
	vec3(0.0, -0.4, 0.0),
	vec3(0.4, 0.4, 0.0),
	vec3(-0.4, 0.4, 0.0)
);

vec3 vertColors[3] = vec3[](
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1)
);
 
void main(){
    // Use '-V' solves undeclared identifier error on gl_VertexIndex
    gl_Position = vec4(vertCoords[gl_VertexIndex], 1.0);
    fragColor   = vertColors[gl_VertexIndex];
}