#pragma once
#include "config.h"

FixedCameraState loadCamState();

void saveCamState(FixedCameraState state);

bool processLoadStoreFile();
