#pragma once

#include "Essentials.h"
#include "IObserver.h"
#include "ISignal.h"

#include <unordered_set>

class Observer : public IObserver {
	DISABLE_COPY(Observer)
public:
	Observer() = default;
	~Observer() override
	{
		for (auto signal : m_signals) {
			signal->unsubscribe(this);
		}
	}

	void addConnectedSignal(ISignal* signal) override
	{
		m_signals.insert(signal);
	}

	void removeConnectedSignal(ISignal* signal) override
	{
		m_signals.erase(signal);
	}

private:
	std::unordered_set<ISignal*> m_signals;
};