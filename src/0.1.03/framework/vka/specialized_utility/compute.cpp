#include "compute.h"
#include <vka/globals.h>
namespace vka
{
	size_t getReduceValueSize(ReduceValueType type)
	{
		switch (type)
		{
			case vka::REDUCE_VALUE_TYPE_FLOAT:
				return sizeof(float);
				break;
			case vka::REDUCE_VALUE_TYPE_UINT32:
				return sizeof(uint32_t);
				break;
			default:
				DEBUG_BREAK;
				return 0;
				break;
		}
	}
	ComputeCmd getCmdReduceVertical(Image src, Buffer dst, uint32_t segmentCount, ReduceOperation op)
	{
	    ComputeCmd cmd(glm::uvec2(src->getExtent2D().width, segmentCount), cVkaShaderPath + "reduce_img_to_buffer_vertical.comp",
	                   {{"FORMAT", getGLSLFormat(src->getFormat())},
			{"REDUCE_OP", static_cast<uint32_t>(op)},
			});
	    cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	    cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		struct PushStruct
	    {
		    uint32_t width;
		    uint32_t height;
		    uint32_t segmentCount;
	    } pc;
		pc.width = src->getExtent2D().width;
		pc.height = src->getExtent2D().height;
	    pc.segmentCount = segmentCount;
	    cmd.pushConstant(&pc, sizeof(PushStruct));
	    return cmd;
	}

	ComputeCmd getCmdReduce(Buffer src, Buffer dst, uint32_t segmentCount, ReduceOperation op, ReduceValueType type)
    {
	    uint32_t inputSize       = src->getSize() / getReduceValueSize(type);
	    uint32_t outputSize      = dst->getSize() / getReduceValueSize(type);
	    uint32_t   invocationCount = outputSize;
	    ComputeCmd cmd(invocationCount, cVkaShaderPath + "reduce_buffer_to_buffer.comp",
	                   {
	                       {"REDUCE_OP", static_cast<uint32_t>(op)},
	                       {"REDUCE_VALUE_TYPE", static_cast<uint32_t>(type)},
	                   });
	    cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	    cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	    struct PushStruct
	    {
		    uint32_t inputSize;
		    uint32_t outputSize;
	    } pc;
	    pc.inputSize  = inputSize;
	    pc.outputSize = outputSize;
	    cmd.pushConstant(&pc, sizeof(PushStruct));
	    return cmd;
    }
    }