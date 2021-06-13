#version 430 core
#define PI 3.141592

out vec4 outColor;

in vec2 uv;
in vec3 outNormal;
in vec3 fragPos;
in mat3 TBN;

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight{
    vec3 position;
    vec3 color;
    float attenuation;
};

struct SpotLight{
    vec3 position;
    vec3 color;
    float attenuation;

    float innerAngle;
    float outerAngle;
};

struct Material{
	bool hasAlbedoMap;
	bool hasRoughnessMap;
	bool hasMetallicMap;
	bool hasNormalMap;
	bool hasAmbientOcclusionMap;

	sampler2D albedoMap;
	vec3 albedo;

	sampler2D roughnessMap;
	float roughness;

	sampler2D metallicMap;
	float metallic;

	sampler2D normalMap;

	sampler2D ambientOcclusionMap;
};

struct PixelMaterial{
	vec3 albedo;
	float roughness;
	float metallic;
	vec3 normal;
	float ao;
    float alpha;
};

uniform vec3 camPos;

uniform Material mat;

uniform samplerCube irradianceMap;
uniform samplerCube prefilteredEnvironmentMap;
uniform sampler2D brdfLutMap;

float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

PixelMaterial getMaterial();
vec3 CalcDirLight(DirLight light, vec3 F0, vec3 viewDir, vec4 lightFragmentPosition, PixelMaterial pbr);
vec3 calcNormalMapping();
vec3 calcFlatNormalMapping();

void main(){
    DirLight sun;
    sun.direction = vec3(.3, -.2, .45);
    sun.color = vec3(4);
    
    
    PixelMaterial pbr = getMaterial();

	//fragment to eye of the camera vector
    vec3 V = normalize(camPos - fragPos);
    vec3 R = reflect(-V, pbr.normal);

	//surface reflection at zero incidence
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, pbr.albedo, pbr.metallic);

	vec3 Lo = vec3(0);

    Lo += CalcDirLight(sun, F0, V, vec4(1), pbr);

	vec3 F = fresnelSchlickRoughness(max(dot(pbr.normal, V), 0), F0, pbr.roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 irradiance = texture(irradianceMap, pbr.normal).rgb;
    vec3 diffuseValue = irradiance * pbr.albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilteredEnvironmentMap, R,  pbr.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLutMap, vec2(max(dot(pbr.normal, V), 0.0), pbr.roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuseValue + specular) * pbr.ao;

    
    vec3 color = (ambient) + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color.rgb, vec3(1.0/2.2));
    outColor = vec4(vec3(color), pbr.alpha);
    
    //outColor = texture(mat.ambientOcclusionMap, uv);
    //outColor.rgb = vec3(mat.hasAmbientOcclusionMap);
}

vec3 gammaCorrect(vec3 value){
    return pow(value, vec3(2.2));
}

float gammaCorrect(float value){
    return pow(value, 2.2);
}

PixelMaterial getMaterial(){
    PixelMaterial ret;

    //albedo
    if(mat.hasAlbedoMap){
    const vec4 tex = texture(mat.albedoMap, uv);
        ret.albedo = gammaCorrect(tex.rgb);
        ret.alpha = tex.a;
    }
    else{
        ret.albedo = mat.albedo;
        ret.alpha = 1;
    }

    //roughness
    if(mat.hasRoughnessMap){
        ret.roughness = gammaCorrect(texture(mat.roughnessMap, uv).r);
    }
    else{
        ret.roughness = mat.roughness;
    }

    //metallic
    if(mat.hasMetallicMap){
        ret.metallic = texture(mat.metallicMap, uv).r;
    }
    else{
        ret.metallic = mat.metallic;
    }

    //normal
    if(mat.hasNormalMap){
        ret.normal = calcNormalMapping();
    }
    else{
        ret.normal = calcFlatNormalMapping();
    }

    //ambientOcclusionMap
    if(mat.hasAmbientOcclusionMap){
        ret.ao = texture(mat.ambientOcclusionMap, uv).r;
    }
    else{
        ret.ao = 1.0;
    }
    return ret;
}

//----------------------------
//    lighting functions
//----------------------------

vec3 CalcDirLight(DirLight light, vec3 F0, vec3 viewDir, vec4 lightFragmentPosition, PixelMaterial pbr){
    vec3 LightDir = normalize(-light.direction);
    // calculate per-light radiance
    vec3 H = normalize(viewDir + LightDir);
    vec3 radiance = light.color;
    
    // cook-torrance brdf
    float NDF = DistributionGGX(pbr.normal, H, pbr.roughness);   
    float G   = GeometrySmith(pbr.normal, viewDir, LightDir, pbr.roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - pbr.metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(pbr.normal, viewDir), 0.0) * max(dot(pbr.normal, LightDir), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);
        
    float NdotL = max(dot(pbr.normal, LightDir), 0.0);  
    
    float shadow = 0;//calcShadow(lightFragPos, pbr.normal, LightDir);

    return (1.0 - shadow) * ((kD * pbr.albedo / PI + specular) * radiance * NdotL);
}

vec3 calcNormalMapping(){
    return normalize(TBN * (texture(mat.normalMap, uv).rgb * 2.0 - 1.0));
}

vec3 calcFlatNormalMapping(){
    return normalize(TBN * vec3(0, 0, 1));
}

//----------------------------------------------
//              pbr functions
//----------------------------------------------

float DistributionGGX(vec3 N, vec3 H, float a)
{
    const float a2     = a*a;
    const float NdotH  = max(dot(N, H), 0.0);
    const float denom  = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), k) * GeometrySchlickGGX(max(dot(N, L), 0.0), k);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  