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
layout(binding = 5) buffer GRID_STORAGE {PinGridEntry sGrid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};


uint getCellIndex(uvec3 idx)
{
	return  (idx.x + idx.y*PIN_GRID_SIZE + idx.z*PIN_GRID_SIZE*PIN_GRID_SIZE)*PINS_PER_GRID_CELL;
}


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

	uvec3 cellID = uvec3(floor(origin*PIN_GRID_SIZE));
	uint gridIdx = getCellIndex(cellID);
	float maxDot = 0.f;
	uint maxDotIdx = gridIdx;
	for(uint i = 0; i < PINS_PER_GRID_CELL; i++)
	{
		Pin pin = sGrid[gridIdx + i].pin;
		vec3 pinOrigin, pinDirection;
		getRay(pin, pinOrigin, pinDirection);
		float dotProd = dot(pinDirection, direction);
		if(dotProd > maxDot)
		{
			maxDot = dotProd;
			maxDotIdx = i + gridIdx;
		}
	}
	uint pinIdx = sGrid[maxDotIdx].data.pinIndex;

	// bin to pin
	vec3 pinOrigin,pinDirection;
	Pin p = sGrid[maxDotIdx].pin;
	getRay(p, pinOrigin, pinDirection);
	vec3 rayDir = pinDirection * sign(dot(direction, pinDirection));
	vec3 rayOrigin = pinOrigin + rayDir*dot(origin-pinOrigin,rayDir);



	// Compute ray segment
	float t = min(unitCubeExitDist(rayOrigin,rayDir), uGui.secRayLength);
	vec3 rayDestination = rayOrigin + t*rayDir;

	// Compute transmittance
	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(rayOrigin, rayDestination, sGaussians[i]), 0.0, 1.0);
	}
	outColor.rgb = vec3(transmittance);
}