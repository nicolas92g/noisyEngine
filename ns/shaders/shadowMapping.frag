#version 430 core

out vec4 color;

in vec2 uv;
in vec3 normal;

uniform sampler2D tex;

void main(){
	color = texture(tex, uv);
	color = vec4(1);
	color.rgb *= max(dot(normalize(normal), normalize(vec3(.4, .9, 0))), 0);
}