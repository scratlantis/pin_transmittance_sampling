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
		ExportTask task;
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