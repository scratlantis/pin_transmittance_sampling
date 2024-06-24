#version 460
#extension GL_GOOGLE_include_directive : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../gaussian.glsl"
// in
layout(location = 0) in vec3 fs_world_pos;
layout(location = 1) in vec3 fs_world_normal;

// out
layout(location = 0) out vec4 outColor;

// descriptors
layout(binding = 0) uniform CAM_CONST {CamConst uCam;};
layout(binding = 1) uniform VIEW_CONST {ViewConst uView;};
layout(binding = 2) uniform GUI_VAR {GuiVar uGui;};
layout(binding = 3) uniform CUBE_TRANSFORM {Transform uCube;};

layout(binding = 4) buffer GAUSSIAN_STORAGE {Gaussian sGaussians[GAUSSIAN_COUNT];};

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

	// Compute ray segment
	vec3 dir = TRANSFORM_DIR(uCube.invMat, L_worldSpace);
	vec3 origin = TRANSFORM(uCube.invMat, fs_world_pos);

	applyJitter(uGui.positionalJitter, uGui.angularJitter, origin, dir);

	float t = min(unitCubeExitDist(origin,dir), uGui.secRayLength);
	vec3 destination = origin + t*dir;

	// Compute transmittance
	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(origin, destination, sGaussians[i]), 0.0, 1.0);
	}
	outColor.rgb = vec3(transmittance);
	//outColor.rgb = vec3(t);
}