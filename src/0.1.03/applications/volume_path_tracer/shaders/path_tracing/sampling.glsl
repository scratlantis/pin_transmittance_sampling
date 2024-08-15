

#include "pt_math.glsl"
#ifndef SAMPLING_H
#define SAMPLING_H

// pbrt

vec2 sampleTriangleBarycentrics(vec2 xi) {
	float su0 = sqrt(xi.x);
	return vec2(1 - su0, xi.y * su0);
}

vec3 sampleCosineWeightedHemisphere(vec2 xi)
{
	float xi_0_sqrt = sqrt(xi.x);
	float phi = 2.0f*PI*xi.y;
	return vec3(cos(phi) * xi_0_sqrt, sin(phi) * xi_0_sqrt, sqrt(1.0f - xi.x));
}

float pdfCosineWeightedHemisphere(vec3 w) {
	return w.z * INV_PI;
}

vec3 evalCosineWeightedHemisphere(vec3 kd) {
	return kd * INV_PI;
}

vec3 sampleUniformSphere(vec2 xi) {
	float theta = 2.0f * PI * xi.x;
	float phi = acos(2.0f * xi.y - 1.0f);
	return vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

// just for consistency

float pdfUniformSphere()
{
	return INV_4PI;
}

vec3 evalUniformSphere(vec3 kd)
{
	return vec3(kd * INV_4PI);
}

#endif