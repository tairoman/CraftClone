#pragma once

#include "Signal.h"

template <typename T>
class Property
{
public:
	Property() = default;
	Property(T value) : m_value(std::move(value)) {}

	void set(T value);
	const T& get() const;

	Signal<T> onChange;
private:
	T m_value;
};

template<typename T>
inline void Property<T>::set(T value)
{
	if (m_value == value) {
		return;
	}

	m_value = std::move(value);
	onChange.trigger(m_value);
}

template<typename T>
inline const T& Property<T>::get() const
{
	return m_value;
}
