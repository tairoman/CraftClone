#include "GameEventDispatcher.h"
#include <SDL2/SDL_timer.h>

GameEventDispatcher::GameEventDispatcher() :
	m_lastTick(SDL_GetTicks())
{
}

void GameEventDispatcher::processLoop()
{
	const auto last = m_lastTick;
	m_lastTick = SDL_GetTicks();

	const auto deltaTime = m_lastTick - last;
	onNewFrame.trigger(deltaTime);
}
