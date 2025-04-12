#version 460 core

struct Particle {
    vec4 x;  // particle position
    vec4 v;  // particle velocity
    float w; // particle inverse mass
    int t;   // particle type. one of HAIR, SOLID, or FLUID
    int pd1, pd2;
};

struct PoreData {
    int startIndex;
    float startStrength;
    int endIndex;
    float endStrength;
    float volume;
    float density;
    int pd1, pd2; // padding
};

struct Rod {
    vec4 q;  // rod orientation
    vec4 v;  // rod angular velocity
    float w; // rod inverse mass
    int pd1, pd2, pd3; // padding
};

struct HairStrand {
    vec4 root;
    int nVertices;
    int startVertexIdx;
    int endVertexIdx;
    int nRods;
    int startRodIdx;
    int endRodIdx;
    float l0;
    int pd; // padding
};

struct BucketData {
    int startIndex;
    int particlesInBucket;
    int nextParticleSlot;
    int pd; // padding
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(std430, binding=1) buffer PredictedPositions {
    vec4 ps[];
};

layout(std430, binding=2) buffer GridStartIndices {
    BucketData startIndices[];
};

layout(std430, binding=3) buffer GridCellEntries {
    int cellEntries[];
};

layout(std430, binding=4) buffer FluidDensities {
    float fluidDensities[];
};

layout(std430, binding=5) buffer Lambdas {
    float lambdas[];
};

layout(std430, binding=6) buffer CurvatureNormals {
    vec4 curvatureNormals[];
};

layout(std430, binding=7) buffer OmegasBuffer {
    vec4 omegas[];
};

layout(std430, binding=8) buffer PorousData {
    PoreData poreData[];
};

layout(std430, binding=9) buffer Rods {
    Rod rods[];
};

layout(std430, binding=10) buffer PredictedOrientations {
    vec4 us[];
};

layout(std430, binding=11) readonly buffer HairStrands {
    HairStrand hairStrands[];
};

layout(std430, binding=12) readonly buffer RestDarbouxVectors {
    vec4 d0s[];
};

layout(std430, binding=13) readonly buffer VertexToStrandMap {
    int vertexStrandMap[];
};

layout(std430, binding=14) readonly buffer RodToStrandMap {
    int rodStrandMap[];
};

layout(location = 0) out vec3 eyeSpacePos;
layout(location = 1) flat out uint particleID;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 3) uniform float nearPlaneHeight;

void main() {
    ps[0];
    startIndices[0];
    cellEntries[0];
    fluidDensities[0];
    lambdas[0];
    curvatureNormals[0];
    omegas[0];
    poreData[0];
    rods[0];
    us[0];
    hairStrands[0];
    d0s[0];
    vertexStrandMap[0];
    rodStrandMap[0];
    particleID = gl_VertexID;
    // particleID = gl_InstanceID + gl_VertexID;
    // particleID = gl_BaseInstance + gl_VertexID;
    // particleID = gl_BaseVertex + gl_VertexID;
	vec3 pos = particles[particleID].x.xyz;
	gl_Position = proj * view * vec4(pos, 1);
    float distToCam = distance(pos, viewPos);
    float pointScale = 1 - (distToCam / 1000);
    pointScale = clamp(pointScale, 0.1, 0.7);
	eyeSpacePos = vec3(view * vec4(pos, 1));
	gl_Position = proj * view * vec4(pos, 1);
	gl_PointSize = (nearPlaneHeight * pointScale) / gl_Position.w;
}
