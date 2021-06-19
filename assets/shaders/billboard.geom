#version 430 core

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 projView;
uniform vec3 cameraPos;
uniform vec3 cameraUp;

out vec2 uv;

void main(){
    vec3 pos = gl_in[0].gl_Position.xyz;
    float scale = gl_in[0].gl_Position.w;
    vec3 toCamera = normalize(cameraPos - pos);
    vec3 upDir = normalize(cameraUp);
    vec3 right = normalize(cross(upDir, toCamera));
    vec3 up = normalize(cross(toCamera, right));

    gl_Position = projView * vec4(pos + right * scale + up * scale, 1.0);
    uv = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = projView * vec4(pos + right * scale + up * -scale, 1.0);
    uv = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = projView * vec4(pos + right * -scale + up * scale, 1.0);
    uv = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = projView * vec4(pos + right * -scale + up * -scale, 1.0);
    uv = vec2(0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}