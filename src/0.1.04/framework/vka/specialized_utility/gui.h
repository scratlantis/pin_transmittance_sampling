#pragma once
#include <vka/core/resources/cachable/RenderPass.h>
#include <vka/core/core_utility/types.h>
#include "GVar.h"

namespace vka
{
void configureGui_Default();
void enableGui(RenderPassDefinition rpDef, uint32_t subpassIdx);
void enableGui();
void cmdRenderGui(CmdBuffer cmdBuf, Image target, float x, float y, float width, float height);
void cmdRenderGui(CmdBuffer cmdBuf, Image target);
void setGuiDimensions(Rect2D<float> rect);



//bool addShaderLog();
//void addPlot(hash_t key);


}        // namespace vka

