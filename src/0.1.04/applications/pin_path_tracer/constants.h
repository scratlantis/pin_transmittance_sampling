#pragma once
#include <vka/vka.h>
using namespace vka;
const float         viewMargin        = 0.001;
const Rect2D<float> viewDimensions    = {0.5, viewMargin, 0.5 - 1.0 * viewMargin, 1.0 - 2.0 * viewMargin};
const Rect2D<float> leftGuiDimensions = {0.f + viewMargin, viewDimensions.y, viewDimensions.x - 2.0 * viewMargin, viewDimensions.height};
const Rect2D<float> topGuiDimensions  = {0, 0, 0.5, viewDimensions.y};

const uint32_t maxIndirectRaysPerBounce = 2;

const uint32_t maxHistogramCount = 10;
const uint32_t maxHistValueCount = 1000000; // = 1M * 4Byte = 4MB

const uint32_t maxPlotCount      = 10;
const uint32_t maxPlotValueCount = 10000;


static uint32_t getLeftHistogramOffset()
{
	uint32_t alignment = getDeviceProperties().limits.minStorageBufferOffsetAlignment;
	return alignUp(maxHistValueCount * 4, alignment)/4;
}

