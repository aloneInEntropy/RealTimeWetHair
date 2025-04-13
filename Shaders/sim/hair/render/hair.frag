#version 460 core

layout(location = 0) out vec4 FragColour;
layout(location = 1) out vec4 DepthColour;

layout(location = 0) in vec3 eyeSpacePos;
layout(location = 1) flat in uint particleID;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in float vWeight;

layout(location = 0) uniform mat4 proj;
layout(location = 2) uniform vec3 viewPos;
layout(location = 4) uniform vec4 colour = vec4(1);
layout(location = 5) uniform vec4 guideColour = vec4(1, 0, 0, 1);
layout(location = 6) uniform float particleRadius;

// todo: darken based on hair vertex "wetness"; reuse a padding value
// todo: scale specular highlight and colour by wetness
// todo: implement some features from https://web.engr.oregonstate.edu/~mjb/cs557/Projects/Papers/HairRendering.pdf

void main() {
    vec4 pixelPos = vec4(eyeSpacePos, 1);
    vec4 clipSpacePos = proj * pixelPos;
    float ndc = clipSpacePos.z / clipSpacePos.w;
    gl_FragDepth = ndc *.5 + .5;
    float outDepth = -pixelPos.z;
	DepthColour = vec4(vec3(outDepth/150), 1);

    // from Mathieu Le Muzic (2012)
    // https://publications.scss.tcd.ie/theses/diss/2012/TCD-SCSS-DISSERTATION-2012-037.pdf
    vec3 T = tangent;
    vec3 L = vec3(0, -1, 0);
    vec3 V = normalize(viewPos - fragPos);

    float tol = dot(T, L);
    float tov = dot(T, V);

    float kd = sin(acos(tol));
    kd = max(kd, 0);
    kd = pow(kd, 10);

    float ks = cos(abs(acos(tol) - acos(-tov)));
    ks = max(ks, 0);
    ks = pow(ks, 100000000);

    vec3 col = colour.xyz;
	FragColour = vec4(col*0.6*vWeight + col*0.5*kd*vWeight + vec3(1)*ks, 1);
	// FragColour = vec4(tangent, 1);
}
