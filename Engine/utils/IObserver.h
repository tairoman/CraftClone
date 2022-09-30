#pragma once

class ISignal;

class IObserver {
public:
	virtual ~IObserver() = default;

	virtual void addConnectedSignal(ISignal* signal) = 0;
	virtual void removeConnectedSignal(ISignal* signal) = 0;
};