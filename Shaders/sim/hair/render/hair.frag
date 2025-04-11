#version 460 core

layout(location = 0) out vec4 FragColour;
layout(location = 1) out vec4 DepthColour;

layout(location = 0) in vec3 eyeSpacePos;
layout(location = 1) flat in uint particleID;

layout(location = 0) uniform mat4 proj;
layout(location = 4) uniform vec4 colour = vec4(1);
layout(location = 5) uniform vec4 guideColour = vec4(1, 0, 0, 1);
layout(location = 6) uniform float particleRadius;

void main() {
	FragColour = colour;

    // calculate normal from texture coordinates
    vec3 N;
    N.xy = gl_PointCoord * 2 - 1;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) discard;   // kill pixels outside circle
    N.z = sqrt(1.0 - r2);

    vec4 pixelPos = vec4(eyeSpacePos + N*particleRadius, 1);
    vec4 clipSpacePos = proj * pixelPos;
    float ndc = clipSpacePos.z / clipSpacePos.w;
    gl_FragDepth = ndc *.5 + .5;
    float outDepth = -pixelPos.z;
	DepthColour = vec4(outDepth/150, 0, 0, 1);
}
