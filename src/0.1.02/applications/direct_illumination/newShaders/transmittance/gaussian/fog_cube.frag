#version 460
#extension GL_GOOGLE_include_directive : enable
#include "../../shaderStructs.glsl"
#include "../../random.glsl"
#include "../../brdf.glsl"
#include "../../ray.glsl"
#include "../gaussian.glsl"
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
layout(binding = 6) buffer GAUSSIAN_STORAGE {Gaussian sGaussians[GAUSSIAN_COUNT];};


vec3 getDirectionalIllum(vec3 dir)
{
	if(uGui.useEnvMap == 1)
	{
		vec2 texCoords = getEnvMapTexCoord( dir);
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


	if(false)
	{
	vec3 scatterOrigin = origin;
	vec3 scatterDir = dir;
	float tScatter = (unitCubeExitDist(scatterOrigin,scatterDir));
	vec3 scatterDestination = scatterOrigin + tScatter*scatterDir;
	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(scatterOrigin, scatterDestination, sGaussians[i]), 0.0, 1.0);
	}
	outColor.rgb = vec3(transmittance);


	float phiOrigin = 0;
	float phiDst = 0;
	evalTransmittanceGaussianSegmentPhi(scatterOrigin, scatterDestination, sGaussians[0], phiOrigin, phiDst);
	vec2 rng = vec2(unormNext(seed), unormNext(seed));
	float p = rng.y * (phiDst - phiOrigin) + phiOrigin;
	float dist = gauss_inv_cdf_approx_Schmeiser(p) * distance(scatterOrigin,scatterDestination)*0.2;
	vec3 target = scatterOrigin + dist*scatterDir;
	float depth = distance(target,uCam.camPos.xyz);
	outColor.rgb = vec3(dist);
	return;
	}


	// Compute transmittance
	outColor.rgb = vec3(0.0);
	for(int i = 0; i < 10; i++)
	{
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
		{
			vec3 scatterOrigin = origin + t*dir;
			vec3 scatterDir = normalize(vec3(0.5) - vec3(unormNext(seed),unormNext(seed),unormNext(seed)));
			applyJitter(uGui.positionalJitter, uGui.angularJitter, scatterOrigin, scatterDir);

			float tScatter = min(unitCubeExitDist(scatterOrigin,scatterDir), uGui.secRayLength);
			vec3 scatterDestination = scatterOrigin + tScatter*scatterDir;
			float transmittance = 1.0;
			for(int i = 0; i < GAUSSIAN_COUNT; i++)
			{
				transmittance *= clamp(1.0-uGui.gaussianWeight*weight*evalTransmittanceGaussianSegment(scatterOrigin, scatterDestination, sGaussians[i]), 0.0, 1.0);
			}
			outColor.rgb += 0.1*transmittance*getDirectionalIllum(scatterDir);

			}
		}
	}


}