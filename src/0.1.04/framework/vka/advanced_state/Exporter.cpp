#include "Exporter.h"
#include <vka/globals.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace vka
{
Exporter::Exporter()
{
	exportTasks.resize(gState.io.imageCount);
}

void Exporter::cmdExport(CmdBuffer cmdBuf, ExportTask task)
{
	if (task.pResource->type() == RESOURCE_TYPE_BUFFER
		&& task.bufferInfo.format == EXPORT_BUFFER_FORMAT_FLOAT
		&& task.targetFormat == EXPORT_FORMAT_CSV)
	{
		Buffer             deviceBuffer = static_cast<Buffer>(task.pResource);
		ExportTaskInternal expTask{};
		expTask.path       = task.path;
		expTask.targetFormat = task.targetFormat;
		expTask.bufferInfo = task.bufferInfo;
		expTask.imageInfo.width = 0;
		expTask.hostBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, deviceBuffer->getSize());
		cmdCopyBuffer(cmdBuf, deviceBuffer, expTask.hostBuffer);
		exportTasks[gState.frame->frameIndex].push_back(expTask);
		return;
	}
	else if (task.pResource->type() == RESOURCE_TYPE_IMAGE
		&& task.targetFormat == EXPORT_FORMAT_PNG)
	{
		Image image = static_cast<Image>(task.pResource);
		if (image->getFormat() == VK_FORMAT_R8G8B8A8_UNORM)
		{
			ExportTaskInternal expTask{};
			expTask.path = task.path;
			expTask.targetFormat = task.targetFormat;
			expTask.imageInfo.width = image->getWidth();
			expTask.imageInfo.height = image->getHeight();
			expTask.hostBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, image->getWidth() * image->getHeight() * 4);
			cmdCopyImageToBuffer(cmdBuf, image, expTask.hostBuffer);
			exportTasks[gState.frame->frameIndex].push_back(expTask);
			return;
		}
	}
	printf("Unsupported export task\n");
}
void Exporter::processExports()
{
	for (auto &task : exportTasks[gState.frame->frameIndex])
	{
		if (task.targetFormat == EXPORT_FORMAT_CSV && task.bufferInfo.format == EXPORT_BUFFER_FORMAT_FLOAT)
		{
			float        *pData  = static_cast<float *>(task.hostBuffer->map());
			std::ofstream file(task.path);
			for (uint32_t i = 0; i < task.hostBuffer->getSize() / sizeof(float); i += task.bufferInfo.rowLength)
			{
				for (uint32_t j = 0; j < task.bufferInfo.rowLength; j++)
				{
					file << pData[i + j];
					if (j < task.bufferInfo.rowLength - 1)
					{
						file << ",";
					}
				}
				file << std::endl;
			}
			file.close();
		}
		else if (task.targetFormat == EXPORT_FORMAT_PNG)
		{
			uint8_t *pData = static_cast<uint8_t *>(task.hostBuffer->map());
			stbi_write_png(task.path.c_str(), task.imageInfo.width, task.imageInfo.height, 4, pData, task.imageInfo.width * 4);
		}
		task.hostBuffer->garbageCollect();
	}
	exportTasks[gState.frame->frameIndex].clear();
}
}        // namespace vka