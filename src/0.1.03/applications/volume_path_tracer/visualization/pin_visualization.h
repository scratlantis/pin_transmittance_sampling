#pragma once
#include <vka/vka.h>
#include <medium/Medium.h>
using namespace vka;
void cmdVisualizePins(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer pinBuffer, Buffer pinState, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout);

void cmdVisualizePinFlux(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer pinBuffer, Buffer pinFlux, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout);


enum PinStateFlags
{
	PIN_STATE_FLAGS_INSIDE_GRID = 1 << 0,
	PIN_STATE_FLAGS_CELL_SELECTED = 1 << 1,
	PIN_STATE_FLAGS_PIN_SELECTED = 1 << 2
};

class PinStateManager
{
	Medium   *pMedium;
	glm::vec3 cursorPos;
	glm::vec2 cursorDirection;

	void resetPinState(CmdBuffer cmdBuf);
	void writeGridPinState(CmdBuffer cmdBuf);
	void writeCursorPinState(CmdBuffer cmdBuf);

  public:
	Buffer    pinState = nullptr;
	Buffer	  pinFlux = nullptr;
	PinStateManager() = default;
	~PinStateManager() = default;
	PinStateManager(Medium* pMedium) :
	    pMedium(pMedium), cursorPos(glm::vec3(0.0f)), cursorDirection(glm::vec2(0.0f)), pinState(nullptr){}
	void update(CmdBuffer cmdBuf);
	bool requiresUpdate();
};