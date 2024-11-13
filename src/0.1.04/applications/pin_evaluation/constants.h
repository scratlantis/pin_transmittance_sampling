#pragma once
#include <vka/vka.h>
using namespace vka;
const float         viewMargin        = 0.001;
const float         topGuiHeight      = 0.07;
const float         bottomGuiHeight   = 0.3;
const float         leftGuiWidth      = 0.4;

const Rect2D<float> viewDimensions      = {leftGuiWidth, topGuiHeight, (1.0 - leftGuiWidth) - viewMargin, (1.0 - topGuiHeight - bottomGuiHeight) - viewMargin};
const Rect2D<float> leftGuiDimensions   = {viewMargin, topGuiHeight, leftGuiWidth - 2.0 * viewMargin, (1.0 - topGuiHeight) - viewMargin};
const Rect2D<float> topGuiDimensions    = {viewMargin, viewMargin, 1.0 - 2.0 * viewMargin, topGuiHeight - 2.0 * viewMargin};
const Rect2D<float> bottomGuiDimensions = {leftGuiWidth, 1.0 - bottomGuiHeight, (1.0 - leftGuiWidth) - viewMargin, bottomGuiHeight - viewMargin};

//const Rect2D<float> viewDimensions      = {0.4, viewMargin, 0.6 - 1.0 * viewMargin, 0.7 - 2.0 * viewMargin};
//const Rect2D<float> leftGuiDimensions   = {0.f + viewMargin, viewDimensions.y, viewDimensions.x - 2.0 * viewMargin, 1.0 - 2.0 * viewMargin};
//const Rect2D<float> topGuiDimensions  = {0, 0, 0.5, viewDimensions.y};
//const Rect2D<float> bottomGuiDimensions = {0.4, 0.7, 0.6 - 1.0 * viewMargin, 0.3 - viewMargin};

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

