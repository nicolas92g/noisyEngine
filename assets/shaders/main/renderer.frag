#version 430 core
#define PI 3.141592

#define MAX_DIR_LIGHTS 5
#define MAX_POINT_LIGHTS 50
#define MAX_SPOT_LIGHTS 50

layout(location = 0) out vec4 outColor;

in vec2 uv;
in vec3 outNormal;
in vec3 fragPos;
in mat3 TBN;
in vec4 lightFragPos;

uniform struct DirLight {
    vec3 direction;
    vec3 color;
} dirLights[MAX_DIR_LIGHTS];

uniform int dirLightNumber;

uniform struct PointLight{
    vec3 position ;
    vec3 color;
    float attenuation ;
} pointLights[MAX_POINT_LIGHTS];

uniform int pointLightNumber;

uniform struct SpotLight{
    vec3 position;
    vec3 color ;
    float attenuation;

    vec3 direction;
    float innerCutOff ;
    float outerCutOff;
} spotLights[MAX_SPOT_LIGHTS];

uniform int spotLightNumber;

uniform struct Material{
	bool hasAlbedoMap;
	bool hasRoughnessMap;
	bool hasMetallicMap;
	bool hasEmissionMap;
	bool hasNormalMap;
	bool hasAmbientOcclusionMap;

	sampler2D albedoMap;
	vec3 albedo;

	sampler2D roughnessMap;
	float roughness;

	sampler2D metallicMap;
	float metallic;

    sampler2D emissionMap;
    float emissionStrength;
    vec3 emission;

	sampler2D normalMap;

	sampler2D ambientOcclusionMap;
} mat;

struct PixelMaterial{
	vec3 albedo;
	float roughness;
	float metallic;
    vec3 emission;
	vec3 normal;
	float ao;
    float alpha;
};

uniform vec3 camPos;

uniform samplerCube irradianceMap;
uniform samplerCube prefilteredEnvironmentMap;
uniform sampler2D brdfLutMap;
uniform sampler2D shadowMap;
uniform bool shadows;
uniform float ambientIntensity = 1;

float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

PixelMaterial getMaterial();
vec3 CalcDirLight(DirLight light, vec3 F0, vec3 viewDir, vec4 lightFragmentPosition, PixelMaterial pbr);
vec3 CalcPointLight(PointLight light, vec3 F0, vec3 fragPos, vec3 viewDir, PixelMaterial pbr);
vec3 CalcSpotLight(SpotLight spotLight, vec3 F0, vec3 fragPos, vec3 viewDir, PixelMaterial pbr);
vec3 calcNormalMapping();
float calcShadow(vec4 lightFP, vec3 normal, vec3 lightDir);

void main(){
    PixelMaterial pbr = getMaterial();

	//fragment to eye of the camera vector
    vec3 V = normalize(camPos - fragPos);
    vec3 R = reflect(-V, pbr.normal);

	//surface reflection at zero incidence
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, pbr.albedo, pbr.metallic);

	vec3 Lo = vec3(0);

    for(int i = 0; i < dirLightNumber; ++i){
        Lo += CalcDirLight(dirLights[i], F0, V, vec4(1), pbr);
    }
    
    for(int i = 0; i < pointLightNumber; ++i){
        Lo += CalcPointLight(pointLights[i], F0, fragPos, V, pbr);
    }
    
    for(int i = 0; i < spotLightNumber; ++i){
        Lo += CalcSpotLight(spotLights[i], F0, fragPos, V, pbr);
    }
    

	vec3 F = fresnelSchlickRoughness(max(dot(pbr.normal, V), 0), F0, pbr.roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 irradiance = texture(irradianceMap, pbr.normal).rgb * ambientIntensity;
    vec3 diffuseValue = irradiance * pbr.albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilteredEnvironmentMap, R,  pbr.roughness * MAX_REFLECTION_LOD).rgb * ambientIntensity;
    vec2 brdf  = texture(brdfLutMap, vec2(max(dot(pbr.normal, V), 0.0), pbr.roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuseValue + specular) * pbr.ao;

    vec3 color = ambient + Lo + pbr.emission;
    outColor = vec4(color, pbr.alpha);//hdr output

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
        ret.roughness = texture(mat.roughnessMap, uv).r;
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

    ///emission
    if(mat.hasEmissionMap){
        ret.emission = texture(mat.emissionMap, uv).rgb * mat.emissionStrength;
    }
    else{
        ret.emission = mat.emission;
    }

    //normal
    if(mat.hasNormalMap){
        ret.normal = calcNormalMapping();
    }
    else{
        ret.normal = normalize(outNormal);
    }

    //ambientOcclusionMap
    if(mat.hasAmbientOcclusionMap){
        ret.ao = texture(mat.ambientOcclusionMap, uv).r;
    }
    else{
        ret.ao = 1;
    }
    return ret;
}

//----------------------------
//    lighting functions
//----------------------------

vec3 CalcDirLight(DirLight light, vec3 F0, vec3 viewDir, vec4 lightFragmentPosition, PixelMaterial pbr){
    vec3 LightDir = light.direction;
    // calculate per-light radiance
    vec3 H = normalize(viewDir + LightDir);
    vec3 radiance = light.color;
    
    // cook-torrance brdf
    float NDF = DistributionGGX(pbr.normal, H, pbr.roughness);   
    float G   = GeometrySmith(pbr.normal, viewDir, LightDir, pbr.roughness);      
    vec3 F    = fresnelSchlick(min(max(dot(H, viewDir), 0.0), 1.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - pbr.metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(outNormal, viewDir), 0.1) * max(dot(pbr.normal, LightDir), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);
        
    float NdotL = max(dot(pbr.normal, LightDir), 0.0);  
    
    float shadow = (shadows) ? calcShadow(lightFragPos, pbr.normal, LightDir) : 0;

    return (1.0 - shadow) * ((kD * pbr.albedo / PI + specular) * radiance * NdotL);
}

vec3 CalcPointLight(PointLight light, vec3 F0, vec3 fragPos, vec3 viewDir, PixelMaterial pbr){
        // calculate per-light radiance
        const vec3 L = normalize(light.position - fragPos);
        const vec3 H = normalize(viewDir + L);
        const float distance    = length(light.position - fragPos);
        const float attenuation = 1.0 / (1 + light.attenuation * distance);
        const vec3 radiance  = light.color * attenuation;
        
        // cook-torrance brdf
        const float NDF = DistributionGGX(pbr.normal, H, pbr.roughness);
        const float G   = GeometrySmith(pbr.normal, viewDir, L, pbr.roughness);      
        const vec3 F    = fresnelSchlick(min(max(dot(H, viewDir), 0.0), 1.0), F0);
        
        const vec3 kS = F;
        const vec3 kD = (vec3(1.0) - kS) * (1.0 - pbr.metallic);
        
        const vec3 numerator    = NDF * G * F;
        const float denominator = 4.0 * max(dot(outNormal, viewDir), 0.1) * max(dot(pbr.normal, L), 0.0);
        const vec3 specularColor = numerator / max(denominator, 0.001) * attenuation;
            
        // add to outgoing radiance Lo
        const float NdotL = max(dot(pbr.normal, L), 0.0);                
        return (kD * pbr.albedo / PI + specularColor) * radiance * NdotL;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 F0, vec3 fragPos, vec3 viewDir, PixelMaterial pbr){
    //create a point light with the spot values
    PointLight spot; spot.position = spotLight.position; spot.color = spotLight.color;
    spot.attenuation = spotLight.attenuation;
    //calculate this point light 
    const vec3 pointLight = CalcPointLight(spot, F0, fragPos, viewDir, pbr);

    //light direction
    const vec3 L = normalize(spotLight.position - fragPos);

    //determine spot 
    const float theta = dot(L, spotLight.direction);
    const float epsilon = spotLight.innerCutOff - spotLight.outerCutOff;
    const float spotStrength =  clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    
    //return the spot color
    return pointLight * spotStrength;
}

vec3 calcNormalMapping(){
    return normalize(TBN * ((texture(mat.normalMap, uv).rgb) * 2.0 - 1.0));
}

float calcShadow(vec4 lightFP, vec3 normal, vec3 lightDir){
    // perform perspective divide
    vec3 projCoords = lightFP.xyz / lightFP.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    //test if the fragment is in the texture and can be determine
    if(projCoords.z > 1.0)
        return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    const float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    const float currentDepth = projCoords.z;
    
    const float bias = .005;//max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            if (currentDepth - bias > pcfDepth){
                shadow += 1.0 / 9.0;
            }
        }    
    }

    //smooth limits
    //const float transitionSize = .05;
    //
    //if(projCoords.y < transitionSize){
    //    shadow *= projCoords.y / transitionSize;
    //}
    //if(projCoords.x < transitionSize){
    //    shadow *= projCoords.x / transitionSize;
    //}
    return shadow;
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