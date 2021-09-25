#pragma once

#include "utils/Signal.h"

class GameEventDispatcher
{
public:
	GameEventDispatcher();

	Signal<uint32_t> onNewFrame;

	void processLoop();

private:
	uint32_t m_lastTick;
};