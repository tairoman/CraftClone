#pragma once

#include <functional>
#include <unordered_map>

#include "IObserver.h"
#include "ISignal.h"

template <typename T>
class Signal : public ISignal
{
	using CallbackT = std::function<void(const T&)>;
public:
	Signal()
	{
		m_callbacks.reserve(5);
	}

	~Signal() override
	{
		for (auto& [observer, _] : m_callbacks) {
			observer->removeConnectedSignal(this);
		}
	}

	void unsubscribe(IObserver* observer) override
	{
		m_callbacks.erase(observer);
	}

	void trigger(const T& value)
	{
		for (auto& [_, func] : m_callbacks) {
			func(value);
		}
	}

	void listen(IObserver& observer, CallbackT callback)
	{
		m_callbacks.insert({ &observer, std::move(callback) });
		observer.addConnectedSignal(this);
	}
private:
	std::unordered_map<IObserver*, CallbackT> m_callbacks;
};

class VoidSignal : public ISignal
{
	using CallbackT = std::function<void()>;
public:
	VoidSignal()
	{
		m_callbacks.reserve(5);
	}

	~VoidSignal() override
	{
		for (auto& [observer, _] : m_callbacks) {
			observer->removeConnectedSignal(this);
		}
	}

	void unsubscribe(IObserver* observer) override
	{
		m_callbacks.erase(observer);
	}

	void trigger()
	{
		for (auto& [_, func] : m_callbacks) {
			func();
		}
	}

	void listen(IObserver& observer, CallbackT callback)
	{
		m_callbacks.insert({ &observer, callback });
		observer.addConnectedSignal(this);
	}
private:
	std::unordered_map<IObserver*, CallbackT> m_callbacks;
};