#version 430 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

uniform mat4 model;
uniform mat4 projView;
uniform bool computeBitangents;

out vec2 uv;
out vec3 outNormal;
out vec3 fragPos;
out mat3 TBN;

void main(){
	gl_Position = projView * model * vec4(inPos, 1);

	uv = inUv;
	outNormal = normalize(vec3(model * vec4(inNormal, 1)));
	fragPos = vec3(model * vec4(inPos, 1));

	vec3 T = normalize(vec3(model * vec4(inTangent, 0.0)));
	vec3 B;
    vec3 N = normalize(vec3(model * vec4(inNormal, 0.0)));

	if(computeBitangents)
	{
		T = normalize(T - dot(T, N) * N);
		B = cross(N, T);
	}
	else
	{
		B = normalize(vec3(model * vec4(inBitangent, 0.0)));
	}
	
	

    TBN = mat3(T, B, N);
}