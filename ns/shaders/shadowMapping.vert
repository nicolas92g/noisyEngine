#version 430 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

uniform mat4 model;
uniform mat4 projView;

out vec2 uv;
out vec3 normal;

void main(){
	gl_Position = projView * model * vec4(inPos, 1);
	uv = inUv;
	normal = inNormal;
}