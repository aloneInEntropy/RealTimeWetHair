#version 460 core
precision highp float;

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 FragColour;

layout (binding = 0) uniform highp sampler2D depthTex; // filtered depth
layout (binding = 1) uniform highp sampler2D normalTex; // filtered normal
layout (binding = 2) uniform highp sampler2D thicknessTex; // volume
layout (binding = 3) uniform highp samplerCube cubemapTex; // skybox
layout (binding = 4) uniform highp sampler2D environmentColourTex; // environment
layout (binding = 5) uniform highp sampler2D environmentDepthTex; // env depth

layout (location = 0) uniform float particleRadius;
layout (location = 1) uniform mat4 proj;
layout (location = 2) uniform float nearPlane;
layout (location = 3) uniform float farPlane;
layout (location = 4) uniform bool showDiffuse = false;
layout (location = 5) uniform vec3 waterCol = vec3(0, 0.2, 0.8) / 10;
layout (location = 6) uniform vec3 attenuationCol = vec3(0.5, 0.2, 0.05);

const float refractiveIndex = 1.33;
const float eta             = 1.0 / refractiveIndex; // Ratio of indices of refraction
const float fresnelPower    = 20.0;
const float F               = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));

vec3 lightDir = -vec3(0, -1, 0);

vec3 uvToEye(vec2 texCoord, float depth) {
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farPlane + nearPlane) / (farPlane - nearPlane) * depth + 2 * farPlane * nearPlane / (farPlane - nearPlane)) / depth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = inverse(proj) * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 computeAttennuation(float thickness) {
    return vec3(exp(-attenuationCol.r * thickness), exp(-attenuationCol.g * thickness), exp(-attenuationCol.b * thickness));
}

void main() {
    float pixelDepth = texture(depthTex, uv).r;
    float bgDepth = texture(environmentDepthTex, uv).r;
    vec3 background = texture(environmentColourTex, uv).xyz;

    if (pixelDepth <= 0 || pixelDepth > 1000) {
        FragColour = vec4(background, 1);
        return;
    }

    // add manual depth test for fluids and environment
    if (pixelDepth > 0 && bgDepth > 0 && pixelDepth > bgDepth) {
        FragColour = vec4(background, 1);
        return;
    }

    vec3 N = vec3(texture(normalTex, uv)); // already normalised
    // diffuse shading
    float diff = max(dot(N, lightDir) * 0.5 + 0.5, 0.0);
    // specular shading
    vec3 viewDir = normalize(uvToEye(uv, pixelDepth));
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = 120;
    float spec = pow(max(dot(N, halfwayDir), 0.0), specularStrength);
    // combine results
    vec3 ambient = 0.2 * waterCol;
    vec3 diffuse = 0.7 * diff * waterCol;
    vec3 specular = vec3(0.3) * spec * specularStrength/100;
    vec3 fluidColour = waterCol + specular;

    // Fresnel Reflection
    float fresnelRatio    = clamp(F + (1.0 - F) * pow((1.0 - dot(viewDir, N)), fresnelPower), 0, 1);
    vec3 reflectionDir   = reflect(-viewDir, N);
    vec3 reflectionColor = texture(cubemapTex, reflectionDir).xyz;

    // Color Attenuation from Thickness (Beer's Law)
    float pixelThickness = texture(thicknessTex, uv).r;
    vec3 colorAttennuation = computeAttennuation(pixelThickness * 10);

    vec3 refractionDir   = refract(-viewDir, N, 1 / refractiveIndex);
    vec3 refractionColor = colorAttennuation * texture(environmentColourTex, uv + refractionDir.xy * pixelThickness * 0.5).xyz;

    fresnelRatio = mix(fresnelRatio, 1, .15);
    vec4 finalColour = vec4((mix(refractionColor, reflectionColor, fresnelRatio) + fluidColour), 0.1 + pixelThickness*10);

    if (showDiffuse) {
        FragColour = vec4(ambient + diffuse + specular, 1);
    } else {
        FragColour = finalColour;
    }
}
