#version 460 core

#define PI 3.14159265358979323846264338327950288

struct PBRMaterial {
  vec3 albedo;
  float metalness;
  float roughness;
};

struct Particle {
    vec4 x;   // particle position
    vec4 v;   // particle velocity
    float w;  // particle inverse mass
    int t;    // particle type. one of HAIR, SOLID, or FLUID
    float d;  // hair wetness for HAIR particles. mass diffusion for FLUID particles. undefined for any PORE particles
    int s;    // strand index for HAIR particles. undefined otherwise
};
layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(location = 0) out vec4 FragColour;
layout(location = 1) out vec4 DepthColour;
in ES_OUT {
    flat uint particleID;
    float vWeight;
    vec3 tangent;
    vec3 eyeSpacePos;
    vec3 fragPos;
} es;

float wetness = 0.1;
float wetnessInv = 10;
float wetnessSub = 0.9;

layout(location = 0) uniform mat4 proj;
layout(location = 2) uniform vec3 viewPos;
layout(location = 4) uniform vec4 colour = vec4(1);
layout(location = 5) uniform float particleRadius;
layout(location = 6) uniform float fresnelPower;
layout(location = 7) uniform vec3 headPos;
layout(location = 8) uniform PBRMaterial pbrMaterial;

vec3 CalcDirLight(vec3 lightDir, vec3 normal, vec3 viewDir);
float chi(float x);
float GGX_D(vec3 normal, vec3 halfwayDir, float roughness);
float GGX_G_Schlick(vec3 normal, vec3 viewDir, float roughness);
float GGX_G(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness);
vec3 fresnel(float cos_theta, vec3 F0);

// // todo: darken based on hair vertex "wetness"; reuse a padding value
// // todo: scale specular highlight and colour by wetness
// // todo: can't change colour back once wet???
// pbr: 
// https://graphicscompendium.com/gamedev/15-pbr
// https://typhomnt.github.io/teaching/ray_tracing/pbr_intro/
// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx

// ! https://www.gdcvault.com/play/1014538/Approximating-Translucency-for-a-Fast
// ^ interesting to mention for improved lighting in the future

void main() {
    vec4 pixelPos = vec4(es.eyeSpacePos, 1);
    vec4 clipSpacePos = proj * pixelPos;
    float ndc = clipSpacePos.z / clipSpacePos.w;
    gl_FragDepth = ndc *.5 + .5;
    float outDepth = -pixelPos.z;
	DepthColour = vec4(vec3(outDepth/150), 1);
    
    wetness = particles[es.particleID].d;
    wetness = clamp(wetness, 0.1, 1);
    wetnessSub = 1 - wetness;
    wetnessInv = 1/wetness;

    // properties
	vec3 norm = normalize(es.fragPos - headPos);
	vec3 viewDir = normalize(viewPos - es.fragPos);

	// directional lights
	vec3 result = pbrMaterial.albedo * 0.2 + CalcDirLight(vec3(-1, -1, 0), norm, viewDir);

	vec3 ev = normalize(es.fragPos - viewPos);
	float ff = abs(dot(ev, norm));
	result += fresnel(ff, vec3(0.05)) * es.vWeight * (outDepth/150) * 0.5;
	/* gamma correction */
	// result /= (result + vec3(1));
	// result = pow(result, vec3(1/2.2));

	FragColour = vec4(result, 1.0);
	// FragColour = vec4(norm, 1.0);
}



// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 lightDir, vec3 N, vec3 V) {
	vec3 L = normalize(-lightDir);
	vec3 H = normalize(L + V);
	vec3 radiance = pbrMaterial.albedo * clamp(wetnessSub, 0, 1);
	float mtl = clamp(pbrMaterial.metalness + 0, 0, 1);
	float rgh = clamp(pbrMaterial.roughness - 0, 0, 1);

	float NoL = max(dot(N, L), 0);
	vec3 F0 = vec3(0.5);
	F0 = mix(F0, pbrMaterial.albedo, mtl);
	float D = GGX_D(N, H, rgh);
	float G = GGX_G(N, V, L, rgh);
	vec3 F = fresnel(max(dot(H, V), 0), F0);

	vec3 BRDF = (D * G * F) / (4.0 * max(dot(N, V), 0.0) * NoL + 0.0001);
	vec3 I = normalize(es.fragPos - viewPos);
	float ior = 1;
	
	vec3 kS = F;
	vec3 kD = (vec3(1) - kS) * ior * ior / /* PI *  */vec3(.5);
    kD = (vec3(1) - kS) * (1 - pbrMaterial.metalness) / /* PI *  */vec3(.5);
	return (kD + BRDF * vec3(0.8)) * radiance * NoL;
}

// normal distribution function (Trowbridge-Reitz GGX)
float GGX_D(vec3 normal, vec3 halfwayDir, float roughness) {
	float a = roughness * roughness;
	float a2 = a*a;
	float HoN = max(dot(normal, halfwayDir), 0);
	float partial_denom = ((HoN * HoN) * (a2 - 1) + 1);
	float denom = PI * partial_denom * partial_denom;
	float D = a2 / denom;
	return D;
}

// geometry function (Schilck-GGX)
float GGX_G_Schlick(vec3 normal, vec3 viewDir, float roughness) {
	float NoV = max(dot(normal, viewDir), 0);
	float r = roughness + 1;
	float k = (r * r) / 8;
	return NoV / ((NoV) * (1-k) + k);
}

// geometry function (Smith-GGX)
float GGX_G(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
	float g1 = GGX_G_Schlick(normal, viewDir, roughness);
	float g2 = GGX_G_Schlick(normal, lightDir, roughness);
	return g1 * g2;
}

// fresnel equation (Schlick's approximation)
vec3 fresnel(float cos_theta, vec3 F0) {
	return F0 + (1-F0) * pow(clamp(1 - cos_theta, 0, 1), fresnelPower * wetnessSub);
}

float chi(float x) {
  return x > 0 ? 1 : 0;
}
