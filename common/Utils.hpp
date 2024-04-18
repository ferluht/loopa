#pragma once

/**
 * @private
 */
float soft_clip(float in);

inline float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t) {
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}