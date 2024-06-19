#pragma once
#include "types.h"
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
#include "cmd_buffer_functionality.h"
#include <vka/core/common.h>

void        vkaSwapBuffers(std::vector<VkaCommandBuffer> cmdBufs);
vka::VkRect2D_OP vkaGetScissorRect(float x, float y, float width = 1.f, float height = 1.f);