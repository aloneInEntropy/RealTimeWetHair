#version 460 core

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 FragColour;

layout (binding = 0) uniform sampler2D sampleTex;
layout (binding = 1) uniform sampler2D sampleTex2;

void main() {
	FragColour = texture(sampleTex, uv);
}
