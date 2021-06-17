#version 430 core

in vec2 uv;

out vec4 outColor;

uniform sampler2D tex;

void main(){
	outColor = texture(tex, uv);
}