#version 330

layout(location = 0) in vec3 inPos;

uniform mat4 projectionView;

void main(){
	gl_Position = projectionView * vec4(inPos, 1);
}