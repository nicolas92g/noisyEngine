#version 430

layout(location = 0) in vec3 inPos;
layout(location = 2) in vec2 inUv;

out vec2 uv;

uniform mat4 projection;
uniform mat4 model;

void main(){
	gl_Position = projection * model * vec4(inPos, 1.0);
	uv = inUv;
}
