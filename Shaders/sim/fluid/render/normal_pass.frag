#version 460 core

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 FragColour;

layout (binding = 0) uniform sampler2D blurredDepthTex;

layout (location = 0) uniform float particleRadius;
layout (location = 1) uniform mat4 proj;
layout (location = 2) uniform float nearPlane;
layout (location = 3) uniform float farPlane;
layout (location = 4) uniform vec2 stride;

vec3 uvToEye(vec2 texCoord, float depth) {
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farPlane + nearPlane) / (farPlane - nearPlane) * depth + 2 * farPlane * nearPlane / (farPlane - nearPlane)) / depth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = inverse(proj) * clipPos;
    return viewPos.xyz / viewPos.w;
}

void main() {
    float pixelWidth  = stride.x;
    float pixelHeight = stride.y;
    float x           = uv.x;
    float xp          = uv.x + pixelWidth;
    float xn          = uv.x - pixelWidth;
    float y           = uv.y;
    float yp          = uv.y + pixelHeight;
    float yn          = uv.y - pixelHeight;

    float depth = texture(blurredDepthTex, vec2(x, y)).r;

    if (depth <= 0 || depth > 1000) {
        FragColour = vec4(0, 1, 0, 1);
        return;
    }

    float depthxp = texture(blurredDepthTex, vec2(xp, y)).r;
    float depthxn = texture(blurredDepthTex, vec2(xn, y)).r;
    float depthyp = texture(blurredDepthTex, vec2(x, yp)).r;
    float depthyn = texture(blurredDepthTex, vec2(x, yn)).r;

    vec3 position   = uvToEye(vec2(x, y), depth);
    vec3 positionxp = uvToEye(vec2(xp, y), depthxp);
    vec3 positionxn = uvToEye(vec2(xn, y), depthxn);
    vec3 dxl        = position - positionxn;
    vec3 dxr        = positionxp - position;

    vec3 dx = (abs(dxr.z) < abs(dxl.z)) ? dxr : dxl;

    vec3 positionyp = uvToEye(vec2(x, yp), depthyp);
    vec3 positionyn = uvToEye(vec2(x, yn), depthyn);
    vec3 dyb        = position - positionyn;
    vec3 dyt        = positionyp - position;

    vec3 dy = (abs(dyt.z) < abs(dyb.z)) ? dyt : dyb;

    // Compute Gradients of Depth and Cross Product Them to Get Normal
    vec3 N = normalize(cross(dx, dy));
    if(isnan(N.x) || isnan(N.y) || isnan(N.y) ||
       isinf(N.x) || isinf(N.y) || isinf(N.z)) {
        N = vec3(0, 0, 1);
    }

    FragColour = vec4(N, 1);
}
