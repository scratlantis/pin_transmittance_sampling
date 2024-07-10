#version 460
#extension GL_GOOGLE_include_directive : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../gaussian.glsl"
#include "../../envmap.glsl"

#ifndef GAUSSIAN_COUNT
#define GAUSSIAN_COUNT 10
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

layout(binding = 8) buffer GAUSSIAN_STORAGE {Gaussian sGaussians[GAUSSIAN_COUNT];};



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

	float t = unitCubeExitDist(origin,dir);
	//t = 10.0;
	vec3 destination = origin + t*dir;


	// Compute transmittance
	outColor.rgb = vec3(0.0);
	for(int i = 0; i < 10; i++)
	{
		float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
		bool hit = false;
		for(int i = 0; i < GAUSSIAN_COUNT; i++)
		{
			 vec2 rng = vec2(unormNext(seed), unormNext(seed));
			 hit = intersect2(origin, destination, sGaussians[i], rng, uGui.gaussianWeight*weight, t) || hit;
		}

	if(!hit)
	{
		outColor.rgb += 0.1*getDirectionalIllum(dir);
	}
	else
	{
		{
			vec3 scatterOrigin = origin + t*dir;
			//vec3 scatterDir = normalize(vec3(0.5) - vec3(unormNext(seed),unormNext(seed),unormNext(seed)));

			float pdf = 1.0;
			vec2 uv = vec2(1.0);
			vec3 scatterDir = sampleDirectionEnvMap(pdf, vec4(unormNext(seed),unormNext(seed),unormNext(seed), unormNext(seed)), uv);


			applyJitter(uGui.positionalJitter, uGui.angularJitter, scatterOrigin, scatterDir);

			float tScatter = min(unitCubeExitDist(scatterOrigin,scatterDir), uGui.secRayLength);
			vec3 scatterDestination = scatterOrigin + tScatter*scatterDir;
			float transmittance = 1.0;
			//for(int i = 0; i < GAUSSIAN_COUNT; i++)
			//{
			//	float density = clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(scatterOrigin, scatterDestination, sGaussians[i]), 0.0, 1.0);
			//	transmittance *= density;
			//}

			for(int i = 0; i < GAUSSIAN_COUNT; i++)
			{
				float density = uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(scatterOrigin, scatterDestination, sGaussians[i]);
				transmittance *= exp(-density);
			}

			outColor.rgb += 0.1*transmittance*texture(sampler2D(envMap, envMapSampler), uv).rgb/pdf;
			}
		}
	}


}