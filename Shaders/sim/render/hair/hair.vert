#version 460 core

struct Particle {
    vec4 x;   // particle position
    vec4 v;   // particle velocity
    float w;  // particle inverse mass
    int t;    // particle type. one of HAIR, SOLID, or FLUID
    float d;  // hair wetness for HAIR particles. mass diffusion for FLUID particles. undefined for any PORE particles
    int s;    // strand index for HAIR particles. undefined otherwise
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
    vec4 q;   // rod orientation
    vec4 v;   // rod velocity
    float w;  // rod inverse mass
    int s;    // rod strand index
    int pd1, pd2; // padding
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

out VS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 eyeSpacePos;
    vec3 fragPos;
} vs_out;

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
    vs_out.particleID = gl_VertexID;

    int strandCountI = hairStrands[particles[gl_VertexID].s].nVertices;
    int strandStartI = hairStrands[particles[gl_VertexID].s].startVertexIdx;
    vs_out.vWeight = (gl_VertexID - strandStartI + 1.f) / float(strandCountI);

	vec3 pos = particles[gl_VertexID].x.xyz;
    vs_out.fragPos = pos;
	gl_Position = vec4(pos, 1);
	vs_out.eyeSpacePos = vec3(view * vec4(pos, 1));
}
