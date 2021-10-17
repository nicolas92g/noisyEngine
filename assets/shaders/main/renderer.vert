#version 430 core
#define ANIMATIONS_MAX_BONES 100

#define DOUBLE 1
#if (DOUBLE)
#define MAT4P dmat4
#define MAT3P dmat3
#define VEC2P dvec2
#define VEC3P dvec3
#define VEC4P dvec4
#else
#define MAT4P mat4
#define MAT3P mat3
#define VEC2P vec2
#define VEC3P vec3
#define VEC4P vec4
#endif

//geometry
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
//animation
layout(location = 5) in ivec4 inBonesIDs;
layout(location = 6) in vec4 inWeights;


uniform MAT4P model;
uniform MAT4P projView;
uniform bool computeBitangents;
uniform mat4 bones[ANIMATIONS_MAX_BONES];

out vec2 uv;
out vec3 outNormal;
flat out VEC3P fragPos;
out mat3 TBN;

//shadows
out vec4 lightFragPos;
uniform mat4 lightSpaceMatrix;

void main(){
	//const mat4 animation = mat4(1);
	//bones[inBonesIDs[0]] * inWeights[0] +
    //bones[inBonesIDs[1]] * inWeights[1] +
    //bones[inBonesIDs[2]] * inWeights[2] +
    //bones[inBonesIDs[3]] * inWeights[3];

	gl_Position = vec4(projView * model * VEC4P(inPos, 1.0));

	uv = inUv;
	outNormal = normalize(inNormal);
	fragPos = VEC3P(model * VEC4P(inPos, 1));

	vec3 T = vec3(normalize(VEC3P(model * VEC4P(inTangent, 0.0))));
	vec3 B;
    vec3 N = vec3(normalize(VEC3P(model * VEC4P(inNormal, 0.0))));

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

	lightFragPos = lightSpaceMatrix * vec4(fragPos, 1);
}