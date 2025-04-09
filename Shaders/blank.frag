#version 460 core

layout(location = 0) out vec4 FragColour;

layout(location = 0) in vec3 fragPos;

layout(location = 2) uniform vec4 colour;

void main() {
	FragColour = colour;
}
