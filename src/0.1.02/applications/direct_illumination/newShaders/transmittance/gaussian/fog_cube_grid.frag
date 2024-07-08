#version 460
#extension GL_GOOGLE_include_directive : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../gaussian.glsl"
#include "../pin.glsl"
#include "../../envmap.glsl"
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

layout(binding = 4) uniform sampler envMapSampler;
layout(binding = 5) uniform texture2D envMap;

layout(binding = 6) buffer ENV_MAP_PDF_X {float envmapPdfX[ENVMAP_PDF_BINS];};
layout(binding = 7) buffer ENV_MAP_PDF_Y {float envmapPdfY[ENVMAP_PDF_BINS];};

layout(binding = 8) buffer GAUSSIAN_STORAGE {Gaussian sGaussians[GAUSSIAN_COUNT];};
layout(binding = 9) buffer GRID_STORAGE {PinGridEntry sGrid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};
uint getCellIndex(uvec3 idx)
{
	return  (idx.x + idx.y*PIN_GRID_SIZE + idx.z*PIN_GRID_SIZE*PIN_GRID_SIZE)*PINS_PER_GRID_CELL;
}


vec3 sampleDirectionEnvMap(inout float weight, vec4 rng, inout vec2 uv)
{
	int x = 0;
	int y = 0;
	float cdf = 0;
	for(; x < ENVMAP_PDF_BINS; x++)
	{
		cdf += envmapPdfX[x];
		if(rng.x < cdf)
		{
			break;
		}
	}
	cdf = 0;
	for(; y < ENVMAP_PDF_BINS; y++)
	{
		cdf += envmapPdfY[y];
		if(rng.y < cdf)
		{
			break;
		}
	}
	
	
	
	
	weight = envmapPdfX[x] * envmapPdfY[y] * float(ENVMAP_PDF_BINS * ENVMAP_PDF_BINS);
	uv = vec2((float(x)+rng.z)/ENVMAP_PDF_BINS, (float(y)+rng.w)/ENVMAP_PDF_BINS);
	return uvToDirection(uv);
}

vec3 getDirectionalIllum(vec3 dir)
{
	if(uGui.useEnvMap == 1)
	{
		vec2 texCoords = sampleSphericalMap( dir);
		vec4 texColor = texture(sampler2D(envMap, envMapSampler), texCoords);
		return texColor.xyz;
	}
	else
	{
		return vec3(1.0);
	}
}
void main()
{
	outColor.a = 1.0;
	outColor.rgb = vec3(0.0);

	uint seed = floatBitsToUint(random(gl_FragCoord.xy));
	seed += uView.frameIdx;
	vec3 world_dir = normalize(fs_world_pos - uCam.camPos.xyz);

	// Compute ray segment
	vec3 dir = TRANSFORM_DIR(uCube.invMat, world_dir);
	vec3 origin = TRANSFORM(uCube.invMat, fs_world_pos);


	float t = unitCubeExitDist(origin,dir);
	vec3 destination = origin + t*dir;

	// Compute transmittance
	outColor.rgb = vec3(0.0);
	for(int i = 0; i < 10; i++)
	{
		
	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	bool hit = false;
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		 vec2 rng = vec2(unormNext(seed), unormNext(seed));
		 hit = intersect(origin, destination, sGaussians[i], rng, uGui.gaussianWeight*weight, t) || hit;
	}

	if(!hit)
	{
		outColor.rgb += 0.1*getDirectionalIllum(dir);
	}
	else
	{
	vec3 scatterOrigin = origin + t*dir;
	float pdf = 1.0;
	vec2 uv = vec2(1.0);
	vec3 scatterDir = sampleDirectionEnvMap(pdf, vec4(unormNext(seed),unormNext(seed),unormNext(seed), unormNext(seed)), uv);
	//vec3 scatterDir = normalize(vec3(0.5) - vec3(unormNext(seed),unormNext(seed),unormNext(seed)));
	float tScatter = min(unitCubeExitDist(scatterOrigin,scatterDir), uGui.secRayLength);
	
	applyJitter(uGui.positionalJitter, uGui.angularJitter, scatterOrigin, scatterDir);
	
	uvec3 cellID = uvec3(floor(scatterOrigin*PIN_GRID_SIZE));
	uint gridIdx = getCellIndex(cellID);
	float maxDot = 0.f;
	uint maxDotIdx = gridIdx;
	for(uint i = 0; i < PINS_PER_GRID_CELL; i++)
	{
		Pin pin = sGrid[gridIdx + i].pin;
		vec3 pinOrigin, pinDirection;
		getRay(pin, pinOrigin, pinDirection);
		float dotProd = dot(pinDirection, scatterDir);
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
	vec3 rayDir = pinDirection * sign(dot(scatterDir, pinDirection));
	vec3 rayOrigin = pinOrigin + rayDir*dot(scatterOrigin-pinOrigin,rayDir);
	float t = min(unitCubeExitDist(rayOrigin,rayDir), uGui.secRayLength);
	vec3 rayDestination = rayOrigin + t*rayDir;
	
	transmittance = 1.0;
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-uGui.gaussianWeight*(tScatter/t)*weight*evalTransmittanceGaussianSegment(rayOrigin, rayDestination, sGaussians[i]), 0.0, 1.0);
	}
	//outColor.rgb += 0.1*transmittance*getDirectionalIllum(scatterDir);
	outColor.rgb += 0.1*transmittance*texture(sampler2D(envMap, envMapSampler), uv).rgb/pdf;
	}
	}


}