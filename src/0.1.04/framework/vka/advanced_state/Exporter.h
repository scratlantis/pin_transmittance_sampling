#pragma once
#include <vka/core/core_common.h>

namespace vka
{
enum ExportFormat
{
	EXPORT_FORMAT_PNG,
	EXPORT_FORMAT_CSV,
};

enum ExportBufferFormat
{
	EXPORT_BUFFER_FORMAT_FLOAT,
};

struct ExportBufferInfo
{
	ExportBufferFormat format;
	uint32_t           rowLength;
};
struct ExportImageInfo
{
	uint32_t width;
	uint32_t height;
};

struct ExportTask
{
	std::string        path;
	Resource          *pResource;
	ExportFormat       targetFormat;
	ExportBufferInfo   bufferInfo;        // only used for buffer resources
};

class Exporter
{
	struct ExportTaskInternal
	{
		std::string      path;
		ExportFormat     targetFormat;
		ExportBufferInfo bufferInfo;
		ExportImageInfo  imageInfo;
		Buffer  hostBuffer;
	};

	std::vector<std::vector<ExportTaskInternal>> exportTasks;

  public:
	Exporter();
	~Exporter() = default;
	void cmdExport(CmdBuffer cmdBuf, ExportTask task);
	void processExports();
};
}        // namespace vka