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

layout(location = 0) out vec3 eyeSpacePos;
layout(location = 1) flat out uint particleID;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 3) uniform float nearPlaneHeight;

void main() {
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
