#ifndef KERNELS_H
#define KERNELS_H

#include "util.h"

float poly6Kernel(vec3 r, float h) {
    float sdst = dot(r, r);
    if (sdst >= h * h) return 0;

    sdst = MAX(sdst, 1e-9f);
    float dff = h * h - sdst;
    float poly6Const = 315.f / (64 * PI * pow(h, 9));
    return poly6Const * dff * dff * dff;
}

vec3 poly6KernelGrad(vec3 r, float h) {
    float sdst = dot(r, r);
    if (sdst >= h * h) return vec3(0);

    float dst = sqrt(sdst);
    float dff = h * h - dst * dst;
    float poly6GradConst = 315.f / (64 * PI * pow(h, 9));
    return dff * dff * poly6GradConst * r;
}

float spikyKernel(vec3 r, float h) {
    float sdst = dot(r, r);
    if (sdst >= h * h) return 0;

    float dst = sqrt(sdst);
    float dff = h - dst;
    float spikyConst = 15.f / (PI * pow(h, 6));
    return dff * dff * dff * spikyConst;
}

vec3 spikyKernelGrad(vec3 r, float h) {
    float sdst = dot(r, r);
    if (sdst >= h * h) return vec3(0);

    float dst = MAX(sqrt(sdst), 1e-9f);
    float dff = h - dst;
    float spikyGradConst = -45.f / (PI * pow(h, 6));  // see https://cs418.cs.illinois.edu/website/text/sph.html
    return spikyGradConst * dff * dff * (r / dst);
}

/* [AAI12, Eq. 2] */
vec3 AI12STKernel(vec3 r, float h) {
    float sdst = dot(r, r);
    float dst = MAX(sqrt(sdst), 1e-9f);
    float splineGradConst = 32 / (PI * pow(h, 9));
    float dff = h - dst;
    float prod = pow(dff, 3) * pow(h, 3);
    if (2 * dst > h && dst <= h) {
        return splineGradConst * prod * (r / dst);
    } else if (dst > 0 && 2 * dst <= h) {
        return splineGradConst * (2 * prod - (pow(h, 6.f) / 64)) * (r / dst);
    } else {
        return vec3(0);
    }
}

/* [AAI12, Eq. 6-7] */
float AIT13SplineKernel(vec3 r, float h) {
    float sdst = dot(r, r);
    float dst = MAX(sqrt(sdst), 1e-9f);
    if (!(2 * dst > h && dst <= h)) return 0;

    float rootVal = (-(4 * sdst) / h) + 6*dst - 2*h;
    float root = pow(rootVal, 1.f/4.f); // quartic root
    float splineGradConst = 0.007 / pow(h, 3.25);
    return splineGradConst * root;
}

#endif /* KERNELS_H */
