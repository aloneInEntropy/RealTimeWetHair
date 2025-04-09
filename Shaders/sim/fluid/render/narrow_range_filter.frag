#version 460 core

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 FragColour;

layout (binding = 0) uniform sampler2D depthTex;

layout (location = 0) uniform float particleRadius;
layout (location = 1) uniform vec2 stride; // 1 / screenSize
layout (location = 2) uniform int filterDim; // 0 for default, 1 for 1D, 2 for 2D
layout (location = 3) uniform int filterRadius; // filter width is 2 * filterRadius + 1
layout (location = 4) uniform int maxFilterSize = 11;
layout (location = 5) uniform float nearPlaneHeight;
layout (location = 6) uniform float thresholdRatio = 1;
layout (location = 7) uniform float clampRatio = .001;

// TODO: comment code with appropriate equations from paper

// [TY18], Eq. 4
// Since the 1D kernel is seperable and the 2D kernel will use magnitude of the difference (dot product), compute the dot product of the x- and y-values of the difference individually
float Gaussian1D(float r, float sigma_i) {
    return exp(-r * r/(sigma_i * sigma_i));
}

// [TY18], Eq. 4
float Gaussian2D(vec2 r, float sigma_i) {
    return exp(-dot(r,  r)/(sigma_i * sigma_i));
}

void ModifiedGaussianFilter(
        inout float sampleDepth, 
        inout float weight, 
        inout float weight_other, 
        inout float upper, 
        inout float lower, 
        float lower_clamp, 
        float threshold) {
    if (sampleDepth > upper) {
        weight = 0;
        weight_other = 0;
    } else {
        if(sampleDepth < lower) {
            sampleDepth = lower_clamp;
        } else {
            upper = max(upper, sampleDepth + threshold); /* [TY18], Eq. 8 */
            lower = min(lower, sampleDepth - threshold); /* [TY18], Eq. 9 */
        }
    }
}

float filter1D(float pixelDepth) {
    if (filterRadius == 0) {
        return pixelDepth;
    }

    float threshold  = particleRadius * thresholdRatio; // sigma
    float ratio      = nearPlaneHeight;
    float K          = filterRadius * ratio * particleRadius;
    int   filterSize = min(maxFilterSize, int(ceil(K / pixelDepth))); // delta

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - particleRadius * clampRatio; // z_i - mu

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec2 sum2  = vec2(pixelDepth, 0);
    vec2 wsum2 = vec2(1, 0);
    vec4 dtc   = vec4(stride.x, stride.y, -stride.x, -stride.y); // depending on the pass direction, this will either be (1/w, 0, -1/w, 0) or (0, 1/h, 0, -1/h)

    vec4  f_tex = uv.xyxy;
    float r     = 0;
    float dr    = dtc.x + dtc.y;

    float upper1 = upper;
    float upper2 = upper;
    float lower1 = lower;
    float lower2 = lower;
    vec2  sampleDepth;
    vec2  w2;

    // start from 1 to avoid counting self
    for (int x = 1; x <= filterSize; ++x) {
        f_tex += dtc;
        r     += dr;

        sampleDepth.x = texture(depthTex, f_tex.xy).r;
        sampleDepth.y = texture(depthTex, f_tex.zw).r;

        w2 = vec2(Gaussian1D(r, two_sigma2));
        ModifiedGaussianFilter(sampleDepth.x, w2.x, w2.y, upper1, lower1, lower_clamp, threshold);
        ModifiedGaussianFilter(sampleDepth.y, w2.y, w2.x, upper2, lower2, lower_clamp, threshold);

        sum2  += sampleDepth * w2;
        wsum2 += w2;
    }

    vec2 filterVal = vec2(sum2.x, wsum2.x) + vec2(sum2.y, wsum2.y);
    return filterVal.x / filterVal.y;
}

float filter2D(float pixelDepth) {
    if (filterRadius == 0) {
        return pixelDepth;
    }

    float threshold  = particleRadius * thresholdRatio;
    float ratio      = nearPlaneHeight;
    float K          = filterRadius * ratio * particleRadius;
    int   filterSize = min(maxFilterSize, int(ceil(K / pixelDepth)));

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - particleRadius * clampRatio;

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec4 f_tex = uv.xyxy;

    vec2 r     = vec2(0, 0);
    vec4 sum4  = vec4(pixelDepth, 0, 0, 0);
    vec4 wsum4 = vec4(1, 0, 0, 0);
    vec4 sampleDepth;
    vec4 w4;

    for (int x = 1; x <= filterSize; ++x) {
        r.x     += stride.x;
        f_tex.x += stride.x;
        f_tex.z -= stride.x;
        vec4 f_tex1 = f_tex.xyxy;
        vec4 f_tex2 = f_tex.zwzw;

        for (int y = 1; y <= filterSize; ++y) {
            f_tex1.y += stride.y;
            f_tex1.w -= stride.y;
            f_tex2.y += stride.y;
            f_tex2.w -= stride.y;

            sampleDepth.x = texture(depthTex, f_tex1.xy).r;
            sampleDepth.y = texture(depthTex, f_tex1.zw).r;
            sampleDepth.z = texture(depthTex, f_tex2.xy).r;
            sampleDepth.w = texture(depthTex, f_tex2.zw).r;

            r.y += stride.y;
            w4   = vec4(Gaussian2D(stride * r, two_sigma2));

            ModifiedGaussianFilter(sampleDepth.x, w4.x, w4.w, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter(sampleDepth.y, w4.y, w4.z, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter(sampleDepth.z, w4.z, w4.y, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter(sampleDepth.w, w4.w, w4.x, upper, lower, lower_clamp, threshold);

            sum4  += sampleDepth * w4;
            wsum4 += w4;
        }
    }

    vec2 filterVal;
    filterVal.x = dot(sum4, vec4(1));
    filterVal.y = dot(wsum4, vec4(1));
    return filterVal.x / filterVal.y;
}

void main() {
    float pixelDepth = texture(depthTex, uv).r;
    float outDepth = 0;
    if (pixelDepth <= 0 || pixelDepth > 1000) {
        outDepth = pixelDepth;
    } else {
        outDepth = (filterDim == 1) ? filter1D(pixelDepth) : filter2D(pixelDepth);
    }
    
    FragColour = vec4(vec3(outDepth), 1);
}
