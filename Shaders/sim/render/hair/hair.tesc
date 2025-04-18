#version 460 core

layout (vertices = 2) out; // the inner line of the spline segment

in VS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 eyeSpacePos;
    vec3 fragPos;
} vs_out[];

out CS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 eyeSpacePos;
    vec3 fragPos;
} cs_out[];

patch out vec4 p_1;
patch out vec4 p_2;
void main() {
    gl_TessLevelOuter[0] = float(1);
    gl_TessLevelOuter[1] = float(8);
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
		p_1 = gl_in[0].gl_Position;
		p_2 = gl_in[3].gl_Position;
		gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position;
	} else if (gl_InvocationID == 1) {
		gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position;
	}

    cs_out[gl_InvocationID].particleID = vs_out[gl_InvocationID].particleID;
    cs_out[gl_InvocationID].vWeight = vs_out[gl_InvocationID].vWeight;
    cs_out[gl_InvocationID].eyeSpacePos = vs_out[gl_InvocationID].eyeSpacePos;
    cs_out[gl_InvocationID].fragPos = vs_out[gl_InvocationID].fragPos;
}
