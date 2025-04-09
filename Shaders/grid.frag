#version 460 core

in vec3 fragPos;
out vec4 FragColour;
uniform vec3 viewPos;
uniform vec4 gridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gridColorThick = vec4(vec3(0.75), 1);

float gGridSize = 400.0; // size of entire grid floor. grid is a sqaure, so this is the length of one side
float gGridMinPixelsBetweenCells = 4.0;
float gGridCellSize = 0.01; // units between smallest cells. this value * gridSize = 1 medium cell size

float log10(float x) {
    float f = log(x) / log(10.0);
    return f;
}

float satf(float x) {
    float f = clamp(x, 0.0, 1.0);
    return f;
}

vec2 satv(vec2 x) {
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}

float max2(vec2 v) {
    float f = max(v.x, v.y);
    return f;
}

void main() {
	vec2 dvx = vec2(dFdx(fragPos.x), dFdy(fragPos.x));
    vec2 dvy = vec2(dFdx(fragPos.z), dFdy(fragPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    float l = length(dudv);

    float LOD = max(0.0, log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

    float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

    dudv *= 4.0;

    vec2 mod_div_dudv = mod(fragPos.xz, GridCellSizeLod0) / dudv;
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    mod_div_dudv = mod(fragPos.xz, GridCellSizeLod1) / dudv;
    float Lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );
    
    mod_div_dudv = mod(fragPos.xz, GridCellSizeLod2) / dudv;
    float Lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    float LOD_fade = fract(LOD);
    vec4 Color;

    if (Lod2a > 0.0) {
        Color = gridColorThick;
        Color.a *= Lod2a;
    } else {
        if (Lod1a > 0.0) {
            Color = mix(gridColorThick, gridColorThin, LOD_fade);
	        Color.a *= Lod1a;
        } else {
            Color = gridColorThin;
	        Color.a *= (Lod0a * (1.0 - LOD_fade));
        }
    }
    
    float OpacityFalloff = (1.0 - satf(length(fragPos.xz - viewPos.xz) / gGridSize));
    Color.a *= OpacityFalloff;
    
    // colour in main axes
    // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
    vec2 crrd = fragPos.xz * gGridMinPixelsBetweenCells;
    vec2 deriv = fwidth(crrd);
    float minz = min(deriv.y, 1);
    float minx = min(deriv.x, 1);
    if (fragPos.x > -minx && fragPos.x < minx) Color.xyz = vec3(0.2, 0.2, 1); // z axis
    if (fragPos.z > -minz && fragPos.z < minz) Color.xyz = vec3(1, 0.2, 0.2); // x axis
    FragColour = Color;
}
