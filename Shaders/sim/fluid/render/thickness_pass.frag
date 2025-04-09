#version 460 core

layout(location = 0) out vec4 FragColour;

layout(location = 0) in vec3 eyeSpacePos;
layout(location = 1) flat in int instanceID;

layout(location = 0) uniform mat4 proj;
layout(location = 5) uniform float particleRadius;

uniform float thicknessStrength = 0.05; // controls how strongly the thickness of each particle is affected by the camera distance

void main() {
    // calculate normal from texture coordinates
    vec3 N;
    N.xy = gl_PointCoord * 2 - 1;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) discard;   // kill pixels outside circle
    N.z = sqrt(1.0 - r2);

    FragColour = vec4(vec3(N.z * thicknessStrength), 1) / 10;

    vec4 pixelPos = vec4(eyeSpacePos + N*particleRadius, 1);
    vec4 clipSpacePos = proj * pixelPos;
    float ndc = clipSpacePos.z / clipSpacePos.w;
    gl_FragDepth = ndc *.5 + .5;
}
