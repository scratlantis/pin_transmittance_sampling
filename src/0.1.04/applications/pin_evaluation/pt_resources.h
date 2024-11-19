#pragma once
#include "config.h"




struct TraceResources
{
	enum TraceResourcesType
	{
		TraceResourcesType_Scene = 1 << 0,
		TraceResourcesType_Medium = 1 << 1,
		TraceResourcesType_MediumInstance = 1 << 2,
	};
	uint32_t   resourcesTypes = 0;
	USceneData sceneData;
	Buffer     mediumInstanceBuffer;
	TLAS       mediumTlas;
	Image      mediumTexture;


	void cmdLoadSceneData(CmdBuffer cmdBuf, IResourcePool *pPool);
	void cmdLoadMedium(CmdBuffer cmdBuf, IResourcePool *pPool);
	void cmdLoadMediumInstances(CmdBuffer cmdBuf, IResourcePool *pPool);

	void cmdLoadAll(CmdBuffer cmdBuf, IResourcePool *pPool);
	void cmdLoadUpdate(CmdBuffer cmdBuf, IResourcePool *pPool, std::vector<bool> settingsChanged);

	void garbageCollect()
	{
		sceneData.garbageCollect();
		mediumInstanceBuffer->garbageCollect();
		mediumTlas->garbageCollect();
		mediumTexture->garbageCollect();
	}
};

