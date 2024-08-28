#pragma once
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{

enum ReduceOperation
{
	REDUCE_OP_AVERAGE
};

enum ReduceValueType
{
	REDUCE_VALUE_TYPE_FLOAT,
	REDUCE_VALUE_TYPE_UINT32
};

size_t getReduceValueSize(ReduceValueType type);

ComputeCmd getCmdReduceVertical(Image src, Buffer dst, uint32_t segmentCount, ReduceOperation op);

ComputeCmd getCmdReduce(Buffer src, Buffer dst, uint32_t segmentCount, ReduceOperation op, ReduceValueType type);

}        // namespace vka