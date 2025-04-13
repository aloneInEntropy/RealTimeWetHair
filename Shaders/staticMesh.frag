#version 460 core

precision highp float;
layout(binding = 0) uniform highp sampler2D diffuseSmpl;
layout(binding = 1) uniform highp sampler2D specularSmpl;
layout(binding = 2) uniform highp sampler2D normalSmpl;

struct Material {
	highp sampler2DArray diffuse;
	highp sampler2DArray specular;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define NR_DIR_LIGHTS 10
#define NR_POINT_LIGHTS 10
#define NR_SPOT_LIGHTS 10

in vec3 FragPos;
in vec2 uv;
in mat3 TBN;
in vec3 oN;
in vec3 eyeCoord;

uniform mat4 proj;
uniform vec3 viewPos;
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform Material material;
uniform int nPointLights = 10;
uniform int nSpotLights = 10;
uniform int nDirLights = 10;
uniform bool useNormalMap = false;

layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 DepthColour;

mat3 tbn = mat3(1);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	// properties
	vec3 N = oN;
	tbn = mat3(1);
	if (useNormalMap) {
		N = vec3(texture(normalSmpl, uv)); // take from texture
		N = normalize(N * 2 - 1); // put into tangent space
		tbn = TBN;
	} else {
		N = oN;
		tbn = mat3(1);
	}
	vec3 tFragPos = tbn * FragPos; // tangent space fragPos
	vec3 tViewPos = tbn * viewPos; // tangent space viewPos
	vec3 viewDir = normalize(tViewPos - tFragPos);

	// directional lights
	vec3 result = vec3(0.0);
	for (int i = 0; i < nDirLights; i++)
	result += CalcDirLight(dirLights[i], N, viewDir);

	// point lights
	for (int i = 0; i < nPointLights; i++)
	result += CalcPointLight(pointLights[i], N, tFragPos, viewDir);

	// spot lights
	for (int i = 0; i < nSpotLights; i++)
	result += CalcSpotLight(spotLights[i], N, tFragPos, viewDir);

	// FragColour = vec4(result, 1.0);
	FragColour = vec4(N, 1.0);

	vec4 pixelPos = vec4(eyeCoord, 1);
	vec4 clipSpacePos = proj * pixelPos;
	float ndc = clipSpacePos.z / clipSpacePos.w;
	gl_FragDepth = ndc *.5 + .5;
	float outDepth = -pixelPos.z;
	DepthColour = vec4(vec3(outDepth/150), 1);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 8);
	// combine results
	vec3 ambient = light.ambient * vec3(texture(diffuseSmpl, uv));
	vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseSmpl, uv));
	vec3 specular = light.specular * spec * vec3(texture(specularSmpl, uv));
	return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightPos = tbn * light.position;
	vec3 lightDir = normalize(lightPos - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 8);
	// attenuation
	float distance = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// combine results
	vec3 ambient = light.ambient * vec3(texture(diffuseSmpl, uv));
	vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseSmpl, uv));
	vec3 specular = light.specular * spec * vec3(texture(specularSmpl, uv));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightPos = tbn * light.position;
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 8);
	float distance = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient = light.ambient * vec3(texture(diffuseSmpl, uv));
	vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseSmpl, uv));
	vec3 specular = light.specular * spec * vec3(texture(specularSmpl, uv));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return ambient + diffuse + specular;
}
