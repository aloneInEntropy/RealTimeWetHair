#version 460 core

struct HairStrand {
    vec4 root;
    int nVertices;
    int startVertexIdx;
    int endVertexIdx;
    int nRods;
    int startRodIdx;
    int endRodIdx;
    float l0;
    int pd;
};

layout(std430, binding = 2) buffer HairStrands {
    HairStrand hairStrands[];
};

layout(std430, binding = 6) buffer VertexToStrandMap {
    int vertexStrandMap[];
};

layout(location = 0) out vec4 FragColour;

layout(location = 0) in vec3 outColour;
layout(location = 1) in vec3 fragPos;
layout(location = 2) flat in uint particleID;

layout(location = 2) uniform vec4 colour = vec4(1);
layout(location = 3) uniform vec4 guideColour = vec4(1, 0, 0, 1);

void main() {
	FragColour = fragPos.y < 0 ? vec4(1, 0, 0, 1) : colour;
}
