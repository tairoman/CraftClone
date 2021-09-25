#include "FpsCounter.h"
#include "GameEventDispatcher.h"

FpsCounter::FpsCounter(GameEventDispatcher& gameEvents)
{
	gameEvents.onNewFrame.listen([this](uint32_t deltaTime) {
		m_framessSinceLastReset++;
		m_timeSinceLastReset += deltaTime;
		if (m_timeSinceLastReset >= 1000) {
			//TODO: Save last m_framesSinceLastReset and only signal when changed?
			onFpsChanged.trigger(m_framessSinceLastReset);
			m_timeSinceLastReset = 0;
			m_framessSinceLastReset = 0;
		}
	});
}

std::optional<uint32_t> FpsCounter::fpsWithoutReset() const
{
	if (m_timeSinceLastReset == 0) {
		return {};
	}
	return m_framessSinceLastReset / m_timeSinceLastReset;
}

std::optional<uint32_t> FpsCounter::fpsWithReset()
{
	if (m_timeSinceLastReset == 0) {
		return {};
	}
	const auto result = m_framessSinceLastReset / m_timeSinceLastReset;
	m_timeSinceLastReset = 0;
	m_framessSinceLastReset = 0;
	return result;
}
