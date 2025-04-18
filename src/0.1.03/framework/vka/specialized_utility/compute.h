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
void       cmdComputeImgPdf(CmdBuffer cmdBuf, Image src, Buffer dst, uint32_t divisionsX, uint32_t divisionsY);
ComputeCmd getCmdNormalizeBuffer(Buffer buf, uint32_t offset, uint32_t segmentSize, uint32_t segmentCount);
ComputeCmd getCmdPlot(Buffer src, Image dst, uint32_t offset, glm::vec3 color);
}        // namespace vka