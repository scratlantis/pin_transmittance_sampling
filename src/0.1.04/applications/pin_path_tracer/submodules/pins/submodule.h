#pragma once
#include <config.h>
using namespace glm;

namespace pins
{
#include <shaders/submodules/pins/interface_structs.glsl>

struct PinUpdateArgs
{
	uint32_t count;
	float    rayMarchStepSize;
	float    writePinStepSize;
	uint32_t posGridSize;
	uint32_t dirGridSize;
	uint32_t executionID;
};

struct PinSampleArgs
{
	uint32_t bitMaskIterations;
	uint32_t posGridSize;
	uint32_t dirGridSize;
};

struct PinArgs
{
	uint32_t count;
	float    rayMarchStepSize;
	float    writePinStepSize;
	uint32_t posGridSize;
	uint32_t dirGridSize;
	uint32_t bitMaskIterations;
	uint32_t executionID;

	PinUpdateArgs getUpdateArgs() const
	{
		PinUpdateArgs args{};
		args.count          = count;
		args.rayMarchStepSize = rayMarchStepSize;
		args.writePinStepSize = writePinStepSize;
		args.posGridSize      = posGridSize;
		args.dirGridSize      = dirGridSize;
		args.executionID      = executionID;
		return args;
	}

	PinSampleArgs getSampleArgs() const
	{
		PinSampleArgs args{};
		args.bitMaskIterations = bitMaskIterations;
		args.posGridSize       = posGridSize;
		args.dirGridSize       = dirGridSize;
		return args;
	}
};


void cmdUpdatePinGrid(CmdBuffer cmdBuf, Buffer pinGridBuf, Image scalarField, PinUpdateArgs args);

void cmdBindPins(ComputeCmd &cmd, Buffer pinGridBuf, PinSampleArgs args);

}        // namespace pins