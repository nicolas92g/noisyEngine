#version 430 core
out vec4 FragColor;

in vec3 localPos;
  
uniform samplerCube environmentMap;
uniform bool hdr; 
  
void main()
{
    vec3 envColor = texture(environmentMap, localPos).rgb;

    if (hdr){
        envColor = envColor / (envColor + vec3(1.0));
        envColor = pow(envColor, vec3(1.0/2.2)); 
    }
  
    FragColor = vec4(envColor, 1.0);
}