#pragma once
#include <vka/vka.h>
using namespace vka;
GVar gvar_use_pins{"show pins", 0, GVAR_ENUM, GVAR_APPLICATION, {"None", "All", "Grid", "Nearest Neighbor 1", "Nearest Neighbor 2"}};

struct AppConfig
{
	uint32_t gaussianCount = 30;
	uint32_t gaussianMargin = 0.2f;
	uint32_t pinsPerGridCell = 20;
	uint32_t pinCountSqrt = 100;
	uint32_t gaussFilterRadius = 4;
};

struct AppData
{
	VkaBuffer viewBuf;
	VkaBuffer gaussianBuf;
	VkaBuffer pinBuf;
	VkaBuffer pinDirectionsBuffer;
	VkaBuffer pinGridBuf;
	VkaBuffer pinTransmittanceBuf;
	VkaBuffer pinVertexBuffer;
	VkaBuffer pinIndexBuffer;
	VkaBuffer pinUsedBuffer;
	void init(vka::IResourcePool* pPool)
	{
		VkaBuffer viewBuf             = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		VkaBuffer gaussianBuf         = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkaBuffer pinBuf              = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkaBuffer pinDirectionsBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkaBuffer pinGridBuf          = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkaBuffer pinTransmittanceBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		VkaBuffer pinVertexBuffer     = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		VkaBuffer pinIndexBuffer      = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		VkaBuffer pinUsedBuffer       = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}

	void update(VkaCommandBuffer cmdBuf, AppConfig config)
	{

	}
};