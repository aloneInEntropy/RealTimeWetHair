#version 460 core

struct Particle {
    vec4 x;  // particle position
    vec4 v;  // particle velocity
    float w; // particle inverse mass
    int t;   // particle type. one of HAIR, PORE, SOLID, or FLUID
    float d; // particle wetness for HAIR particles. undefined for any non-HAIR particles
    int pd;  // padding
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(location = 0) out vec3 eyeSpacePos;
layout(location = 1) flat out int globalParticleID;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 3) uniform float nearPlaneHeight;
layout(location = 4) uniform int startIdx;

void main() {
    globalParticleID = startIdx + gl_InstanceID;
    vec3 pos = particles[globalParticleID].x.xyz;
    float distToCam = distance(pos, viewPos);
    float pointScale = 1 - (distToCam / 1000);
    pointScale = clamp(pointScale, 0.1, 0.7);
	eyeSpacePos = vec3(view * vec4(pos, 1));
	gl_Position = proj * view * vec4(pos, 1);
	gl_PointSize = (nearPlaneHeight * pointScale) / gl_Position.w;
}
