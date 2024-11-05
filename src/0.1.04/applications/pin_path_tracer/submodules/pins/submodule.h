#pragma once
#include <config.h>
using namespace glm;

namespace pins
{
//#include <shaders/submodules/pins/interface_structs.glsl>

struct PinUpdateArgs
{
	uint32_t count;
	bool     updateAll;
	float    rayMarchStepSize;
	float    writePinStepSize;
	uint32_t posGridSize;
	uint32_t dirGridSize;
	uint32_t executionID;
	uint32_t bitMaskSize;
};

struct PinSampleArgs
{
	uint32_t bitMaskIterations;
	uint32_t posGridSize;
	uint32_t dirGridSize;
	bool	disableBitmaskSampling; // For debugging
	uint32_t bitMaskSize;
};

struct PinArgs
{
	uint32_t count;
	bool     updateAll;
	float    rayMarchStepSize;
	float    writePinStepSize;
	uint32_t posGridSize;
	uint32_t dirGridSize;
	uint32_t bitMaskIterations;
	uint32_t bitMaskSize;
	uint32_t executionID;
	bool     disableBitmaskSampling;

	PinUpdateArgs getUpdateArgs() const
	{
		PinUpdateArgs args{};
		args.count          = count;
		args.updateAll      = updateAll;
		args.rayMarchStepSize = rayMarchStepSize;
		args.writePinStepSize = writePinStepSize;
		args.posGridSize      = posGridSize;
		args.dirGridSize      = dirGridSize;
		args.executionID      = executionID;
		args.bitMaskSize      = bitMaskSize;
		if (disableBitmaskSampling)
		{
			args.count = 0;
		}
		return args;
	}

	PinSampleArgs getSampleArgs() const
	{
		PinSampleArgs args{};
		args.bitMaskIterations      = bitMaskIterations;
		args.posGridSize            = posGridSize;
		args.dirGridSize            = dirGridSize;
		args.disableBitmaskSampling = disableBitmaskSampling;
		args.bitMaskSize            = bitMaskSize;
		return args;
	}
};


void cmdUpdatePinGrid(CmdBuffer cmdBuf, Buffer pinGridBuf, Image scalarField, PinUpdateArgs args);

void cmdBindPins(ComputeCmd &cmd, Buffer pinGridBuf, PinSampleArgs args);

}        // namespace pins