#version 460 core

struct Particle {
    vec4 x;  // particle position
    vec4 v;  // particle velocity
    float w; // particle inverse mass
    int t;   // particle type. one of HAIR, PORE, SOLID, or FLUID
    float d; // particle wetness for HAIR particles. undefined for any non-HAIR particles
    int pd;  // padding
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

out VS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 tangent;
    vec3 eyeSpacePos;
    vec3 fragPos;
} vs_out;
// layout(location = 0) out vec3 eyeSpacePos;
// layout(location = 1) flat out uint particleID;
// layout(location = 2) out vec3 tangent;
// layout(location = 3) out vec3 fragPos;
// layout(location = 4) out float vWeight;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 3) uniform float nearPlaneHeight;

// Cast a quaternion `q` to a 3x3 matrix. Taken from GLM's implementation
mat3 toMat3(vec4 q) {
    mat3 m = mat3(1);
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    m[0][0] = 1 - 2 * (y*y + z*z);
    m[0][1] = 2 * (x*y + z*w);
    m[0][2] = 2 * (x*z - w*y);
    m[1][0] = 2 * (x*y - z*w);
    m[1][1] = 1 - 2 * (x*x + z*z);
    m[1][2] = 2 * (y*z + x*w);
    m[2][0] = 2 * (x*z + y*w);
    m[2][1] = 2 * (y*z - x*w);
    m[2][2] = 1 - 2 * (x*x + y*y);
    
    return m;
}

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
    vs_out.particleID = gl_VertexID;
    vec3 up = vec3(0, 1, 0);
    if (gl_VertexID == hairStrands[vertexStrandMap[gl_VertexID]].endVertexIdx) {
        // copy previous tangent
        vs_out.tangent = normalize(rods[(gl_VertexID - 1) - vertexStrandMap[(gl_VertexID - 1)]].q.xyz);
    } else {
        vs_out.tangent = normalize(rods[gl_VertexID - vertexStrandMap[gl_VertexID]].q.xyz);
    }
    int strandCountI = hairStrands[vertexStrandMap[gl_VertexID]].nVertices;
    int strandStartI = hairStrands[vertexStrandMap[gl_VertexID]].startVertexIdx;
    vs_out.vWeight = (gl_VertexID - strandStartI + 1.f) / float(strandCountI);
    // particleID = gl_InstanceID + gl_VertexID;
    // particleID = gl_BaseInstance + gl_VertexID;
    // particleID = gl_BaseVertex + gl_VertexID;
	vec3 pos = particles[gl_VertexID].x.xyz;
    vs_out.fragPos = pos;
	gl_Position = vec4(pos, 1);
    // float distToCam = distance(pos, viewPos);
    // float pointScale = 1 - (distToCam / 1000);
    // pointScale = clamp(pointScale, 0.1, 0.7);
	vs_out.eyeSpacePos = vec3(view * vec4(pos, 1));
	// gl_PointSize = (nearPlaneHeight * pointScale) / gl_Position.w;
}
