#version 460
#extension GL_GOOGLE_include_directive : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../../envmap.glsl"
#include "../pin.glsl"

#ifndef PIN_GRID_SIZE
#define PIN_GRID_SIZE 10
#endif
#ifndef PINS_PER_GRID_CELL
#define PINS_PER_GRID_CELL 10
#endif
#ifndef PIN_TRANSMITTANCE_STEPS
#define PIN_TRANSMITTANCE_STEPS 16
#endif
#ifndef PIN_COUNT
#define PIN_COUNT 16
#endif

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

layout(binding = 8) uniform texture3D volumeData;
layout(binding = 9) uniform sampler volumeDataSampler;
layout(binding = 10) buffer GRID_STORAGE {PinGridEntry sGrid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};
layout(binding = 11) buffer PIN_TRANSMITTANCE { float pinTransmittance[PIN_TRANSMITTANCE_STEPS*PIN_COUNT]; };
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
	if(uGui.useEnvMap != 0)
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
	seed+= uView.frameIdx;

	vec3 world_dir = normalize(fs_world_pos - uCam.camPos.xyz);

	// Compute ray segment
	vec3 dir = TRANSFORM_DIR(uCube.invMat, world_dir);
	vec3 origin = TRANSFORM(uCube.invMat, fs_world_pos);

	float rayLength = unitCubeExitDist(origin,dir);
	//t = 10.0;
	vec3 destination = origin + rayLength*dir;

	float weight = 1.0;
	// Compute transmittance
	outColor.rgb = vec3(0.0);
	float stepSize = 0.1;
	for(int i = 0; i < 10; i++)
	{
		bool hit = false;
		float depth = unormNext(seed);
		float transmittance = 1.0;
		float z = 0.0;
		vec3 lastSamplePos = origin;
		for(; z < rayLength; z+=stepSize*unormNext(seed))
		{
			vec3 samplePos = origin + z*dir;
			float density = weight*distance(samplePos,lastSamplePos)*texture(sampler3D(volumeData, volumeDataSampler), samplePos).r;
			transmittance*=exp(-density);
			if(depth > transmittance)
			{
				hit = true;
				break;
			}
			lastSamplePos = samplePos;
		}
		float t = z;


	if(!hit)
	{
		outColor.rgb += 0.1*getDirectionalIllum(dir);
	}
	else
	{
		{
			vec3 scatterOrigin = origin + t*dir;
			float pdf = 1.0;
			vec2 uv = vec2(1.0);
			vec3 scatterDir = sampleDirectionEnvMap(pdf, vec4(unormNext(seed),unormNext(seed),unormNext(seed), unormNext(seed)), uv);
			applyJitter(uGui.positionalJitter, uGui.angularJitter, scatterOrigin, scatterDir);
			float tScatter = min(unitCubeExitDist(scatterOrigin,scatterDir), uGui.secRayLength);
			vec3 scatterDestination = scatterOrigin + tScatter*scatterDir;

			vec3 jitteredDir = scatterDir;
			vec3 jitteredOrigin = scatterOrigin;
			applyJitter(uGui.positionalJitter, uGui.angularJitter, jitteredOrigin, jitteredDir);
			jitteredOrigin = clamp(vec3(0.0),vec3(0.9999), jitteredOrigin);
			
			// bin to pin
			uvec3 cellID = uvec3(floor(jitteredOrigin*PIN_GRID_SIZE));
			uint gridIdx = getCellIndex(cellID);
			float maxDot = 0.f;
			uint maxDotIdx = gridIdx;
			for(uint i = 0; i < PINS_PER_GRID_CELL; i++)
			{
				Pin pin = sGrid[gridIdx + i].pin;
				vec3 pinOrigin, pinDirection;
				getRay(pin, pinOrigin, pinDirection);
				float dotProd = dot(pinDirection, jitteredDir);
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


			vec3 pinEntry, pinExit;
			unitCubeIntersection(pinOrigin, pinDirection, pinEntry, pinExit);
			float valStart, valEnd;
			projectRaySegment(pinEntry, pinExit, scatterOrigin, scatterDestination, valStart, valEnd);

			valStart = clamp(valStart, 0.0, 0.9999);
			valEnd = clamp(valEnd, 0.0, 0.9999);

			valStart*=float(PIN_TRANSMITTANCE_STEPS);
			valEnd *= float(PIN_TRANSMITTANCE_STEPS);

			int rightIndexStart = int(valStart);
			int rightIndexEnd = int(valEnd);
			float alphaStart = valStart - floor(valStart);
			float alphaEnd = valEnd - floor(valEnd);
			uint indexOffset = pinIdx*PIN_TRANSMITTANCE_STEPS;

			float cumulativDensityStart, cumulativDensityEnd;
			if(rightIndexStart == 0)
			{
				cumulativDensityStart = mix(0.0, pinTransmittance[indexOffset + rightIndexStart], alphaStart);
			}
			else
			{
				cumulativDensityStart = mix(pinTransmittance[indexOffset + rightIndexStart-1], pinTransmittance[indexOffset + rightIndexStart], alphaStart);
			}
			if(rightIndexEnd == 0)
			{
				cumulativDensityEnd = mix(0.0, pinTransmittance[indexOffset + rightIndexEnd], alphaEnd);
			}
			else
			{
				cumulativDensityEnd = mix(pinTransmittance[indexOffset + rightIndexEnd-1], pinTransmittance[indexOffset + rightIndexEnd], alphaEnd);
			}

			float cumulativDensity = abs(cumulativDensityStart - cumulativDensityEnd);
			cumulativDensity*=distance(pinEntry, pinExit);
			float coef = 1.0/dot(pinDirection,scatterDir);
			coef = clamp(coef, 1.0, 10.0);
			cumulativDensity*= coef;

			float transmittance = exp(-cumulativDensity);





			//vec3 rayDir = pinDirection * sign(dot(scatterDir, pinDirection));
			//vec3 rayOrigin = pinOrigin + rayDir*dot(scatterOrigin-pinOrigin,rayDir);
			//float t = min(unitCubeExitDist(rayOrigin,rayDir), uGui.secRayLength);
			//vec3 rayDestination = rayOrigin + t*rayDir;
			//transmittance = 1.0;
			//vec3 lastSamplePos = rayOrigin;
			//for(float z = 0.0; z < tScatter; z+=stepSize*unormNext(seed))
			//{
			//	vec3 samplePos = rayOrigin + z*rayDir;
			//	float density = weight*distance(samplePos,lastSamplePos)*texture(sampler3D(volumeData, volumeDataSampler), samplePos).r;
			//	transmittance*=exp(-density);
			//	lastSamplePos = samplePos;
			//}

			


			outColor.rgb += 0.1*transmittance*texture(sampler2D(envMap, envMapSampler), uv).rgb/pdf;

			}
		}
	}


}