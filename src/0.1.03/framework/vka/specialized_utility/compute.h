#pragma once
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{

enum ReduceOperation
{
	REDUCE_OP_AVERAGE,
	REDUCE_OP_IMAGE_PDF
};

enum ReduceValueType
{
	REDUCE_VALUE_TYPE_FLOAT,
	REDUCE_VALUE_TYPE_UINT32
};

size_t getReduceValueSize(ReduceValueType type);

ComputeCmd getCmdReduceVertical(Image src, Buffer dst, uint32_t segmentCount, ReduceOperation op);

ComputeCmd getCmdReduce(Buffer src, Buffer dst, uint32_t segmentCount, ReduceOperation op, ReduceValueType type);

ComputeCmd getCmdDownSample(Image src, Image dst, ReduceOperation op);
}        // namespace vka