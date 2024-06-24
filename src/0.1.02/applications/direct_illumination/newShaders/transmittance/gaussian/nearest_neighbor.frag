#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_control_flow_attributes : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../gaussian.glsl"
#include "../pin.glsl"


// in
layout(location = 0) in vec3 fs_world_pos;
layout(location = 1) in vec3 fs_world_normal;
layout(location = 2) flat in int fs_instance_id;

// out
layout(location = 0) out vec4 outColor;

// descriptors
layout(binding = 0) uniform CAM_CONST {CamConst uCam;};
layout(binding = 1) uniform VIEW_CONST {ViewConst uView;};
layout(binding = 2) uniform GUI_VAR {GuiVar uGui;};
layout(binding = 3) uniform CUBE_TRANSFORM {Transform uCube;};

layout(binding = 4) buffer GAUSSIAN_STORAGE {Gaussian sGaussians[GAUSSIAN_COUNT];};
layout(binding = 5) buffer PIN_STORAGE {Pin sPins[PIN_COUNT];};
// defines: PIN_COUNT,PIN_COUNT_SQRT,METRIC_ANGLE_DISTANCE/METRIC_DISTANCE_DISTANCE





void main()
{
	outColor.a = 1.0;
	outColor.rgb = vec3(0.0);
	uint seed = floatBitsToUint(random(gl_FragCoord.xy));

	// Sample L in tangent space
	vec2 xi = {unormNext(seed), unormNext(seed)};
	vec3 L_tangentSpace = sampleLampert(xi);
	mat3 TNB = getTangentBase(fs_world_normal);
	vec3 L_worldSpace = (normalize(TNB*L_tangentSpace));

	// Transform to cube space
	vec3 direction = TRANSFORM_DIR(uCube.invMat, L_worldSpace);
	vec3 origin = TRANSFORM(uCube.invMat, fs_world_pos);

	applyJitter(uGui.positionalJitter, uGui.angularJitter, origin, direction);

	float tOriginal = min(unitCubeExitDist(origin,direction), uGui.secRayLength);
	vec3 destination = origin + tOriginal*direction;

	float maxMetric = 0.0;
	uint maxMetricIdx = 0;

	uint offset = fs_instance_id * PIN_COUNT_SQRT;
	[[unroll]]
	for(uint i = 0; i < PIN_COUNT_SQRT; i++)
	{
		vec3 pinOrigin, pinDirection;
		Pin p = sPins[i+offset];
		getRay(p, pinOrigin, pinDirection);

		vec3 closestPoint = pinOrigin + pinDirection*dot(origin-pinOrigin,pinDirection);
		float dist = distance(closestPoint, origin);
		float invDist = 1.0/(dist+1.0);
		float metric = 0;

		#ifdef METRIC_ANGLE_DISTANCE
		float dotProd = abs(dot(direction, pinDirection));
		metric = mix(dotProd, invDist, uGui.pinSelectionCoef);
		#endif

		#ifdef METRIC_DISTANCE_DISTANCE
		vec3 closestPointEnd = pinOrigin + pinDirection*dot(destination-pinOrigin,pinDirection);
		float distEnd = distance(closestPointEnd, destination);
		float invDistEnd = 1.0/(distEnd+1.0);
		metric = mix(invDistEnd, invDist, uGui.pinSelectionCoef);
		#endif

		if(metric > maxMetric)
		{
			maxMetric = metric;
			maxMetricIdx = i+offset;
		}
	}
	//outPinId = maxMetricIdx+1;
	gl_FragDepth = 1.0-maxMetric;

	// bin to pin
	vec3 pinOrigin,pinDirection;
	Pin p = sPins[maxMetricIdx];
	getRay(p, pinOrigin, pinDirection);

	vec3 rayDir = pinDirection * sign(dot(direction, pinDirection));
	vec3 rayOrigin = pinOrigin + rayDir*dot(origin-pinOrigin,rayDir);



	// Compute ray segment
	float t = min(unitCubeExitDist(rayOrigin,rayDir), uGui.secRayLength);
	vec3 rayDestination = rayOrigin + t*rayDir;

	if(tOriginal < EPSILON)
	{
		outColor.rgb = vec3(0.0);
		return;
	}

	// Compute transmittance
	float transmittance = 1.0;
	float weight = clamp((tOriginal/t) * 10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(rayOrigin, rayDestination, sGaussians[i]), 0.0, 1.0);
	}
	outColor.rgb = vec3(transmittance);
	//outColor.rgb = vec3(t);
}