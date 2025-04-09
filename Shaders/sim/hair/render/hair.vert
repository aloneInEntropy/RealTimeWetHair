#version 460 core

struct Particle {
    vec4 x;  // particle position
    vec4 v;  // particle velocity
    float w; // particle inverse mass
    int t;   // particle type. one of HAIR, SOLID, or FLUID
    int pd1, pd2;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(location = 0) out vec3 outColour;
layout(location = 1) out vec3 fragPos;
layout(location = 2) flat out uint particleID;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
// layout(location = 2) uniform mat4 view;

void main() {
	outColour = vec3(1);
    particleID = gl_VertexID;
    // particleID = gl_InstanceID + gl_VertexID;
    // particleID = gl_BaseInstance + gl_VertexID;
    // particleID = gl_BaseVertex + gl_VertexID;
	fragPos = vec3(mat4(1) * vec4(particles[particleID].x.xyz, 1.0));
    vec4 outPos = proj * view * vec4(fragPos, 1);
	gl_Position = outPos;
}
