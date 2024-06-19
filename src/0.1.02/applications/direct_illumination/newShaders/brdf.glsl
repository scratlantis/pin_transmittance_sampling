#include "macros.glsl"

vec3 sampleLampert(vec2 xi)
{
	float xi_0_sqrt = sqrt(xi.x);
	float phi = 2.0f*PI*xi.y;
	return vec3(cos(phi) * xi_0_sqrt, sin(phi) * xi_0_sqrt, sqrt(1.0f - xi.x));
}

mat3 getTangentBase(vec3 normal)
{
	vec3 tangent;
	if(normal.x > 0.5) tangent = cross(normal, vec3(0.0,1.0,0.0));
	else tangent = cross(normal, vec3(1.0,0.0,0.0));

	vec3 N = normalize(normal);
	vec3 T = normalize(tangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = -normalize(cross(N, T));
	return mat3(T,B,N);
}