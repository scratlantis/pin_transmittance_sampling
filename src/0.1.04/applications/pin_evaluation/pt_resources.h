#pragma once
#include "config.h"




struct TraceResources
{
	USceneData sceneData;
	Buffer     mediumInstanceBuffer;
	TLAS       mediumTlas;
	Image      mediumTexture;

	void garbageCollect()
	{
		sceneData.garbageCollect();
		mediumInstanceBuffer->garbageCollect();
		mediumTlas->garbageCollect();
		mediumTexture->garbageCollect();
	}
};

TraceResources cmdLoadResources(CmdBuffer cmdBuf, IResourcePool *pPool);
