#pragma once

#include <cstdint>
#include <optional>

#include "utils/Signal.h"
#include "utils/Observer.h"

class GameEventDispatcher;

class FpsCounter
{
public:
	explicit FpsCounter(GameEventDispatcher& gameEvents);

	std::optional<uint32_t> fpsWithoutReset() const;
	std::optional<uint32_t> fpsWithReset();

	Signal<uint32_t> onFpsChanged;

private:
	Observer m_observer;
	uint32_t m_framessSinceLastReset = 0;
	uint32_t m_timeSinceLastReset = 0;
};