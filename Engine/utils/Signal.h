#pragma once

#include <functional>
#include <vector>

template <typename T>
class Signal
{
	using CallbackT = std::function<void(const T&)>;
public:
	Signal()
	{
		m_callbacks.reserve(5);
	}

	void trigger(const T& value);
	void listen(CallbackT callback);
private:
	std::vector<CallbackT> m_callbacks;
};

template<typename T>
inline void Signal<T>::trigger(const T& value)
{
	for (auto& c : m_callbacks) {
		c(value);
	}
}

template<typename T>
inline void Signal<T>::listen(CallbackT callback)
{
	m_callbacks.push_back(std::move(callback));
}
