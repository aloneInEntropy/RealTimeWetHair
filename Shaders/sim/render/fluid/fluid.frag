#version 460 core

layout(location = 0) out vec4 FragColour;

layout(location = 0) in vec3 eyeSpacePos;
layout(location = 1) flat in int instanceID;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 5) uniform float particleRadius;
layout(location = 6) uniform bool showOutline;
layout(location = 7) uniform bool showLighting;
layout(location = 8) uniform bool showBounds;

struct Particle {
    vec4 x;   // particle position
    vec4 v;   // particle velocity
    float w;  // particle inverse mass
    int t;    // particle type. one of HAIR, SOLID, or FLUID
    float d;  // hair wetness for HAIR particles. mass diffusion for FLUID particles. undefined for any PORE particles
    int s;    // strand index for HAIR particles. undefined otherwise
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

// https://mmmovania.blogspot.com/2011/01/point-sprites-as-spheres-in-opengl33.html
void main() {
    float op = 1;

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
    float t = showLighting ? 0 : length(particles[instanceID].v.xyz)/10;

    // calculate lighting
	vec3 lightDir = vec3(0, -1, 0);
    float diffuse = showLighting ? max(0.0, dot(N, lightDir)) : 0;
    vec4 ambient = mix(vec4(0, 0.2, 0.8, op), vec4(0.7, 0.7, 1, op), t);
    vec4 col = vec4(1) * diffuse + ambient;

    FragColour = col;
}
