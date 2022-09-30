#pragma once

class IObserver;

class ISignal {
public:
	virtual ~ISignal() = default;

	virtual void unsubscribe(IObserver* observer) = 0;
};