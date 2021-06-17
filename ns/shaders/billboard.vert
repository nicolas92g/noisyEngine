#version 430 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUv;

uniform mat4 model;
uniform mat4 projView;

uniform vec3 cameraRight;
uniform vec3 cameraUp;

out vec2 uv;

void main(){
	vec3 pos = cameraRight * inPos.x + cameraUp * inPos.y;

	gl_Position = projView * model * vec4(pos, 1.0);
	uv = inUv;
}