#pragma once

#include "Event.h"
#include "EventQueue.h"

#include "../utils/Signal.h"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

class EventThread
{
public:
	EventThread();
	virtual ~EventThread() = default;

	void stop();
	void join();
	
	void pushEvent(std::unique_ptr<Event> ev);

	virtual void onStart() {}
	virtual void onFinish() {}
	virtual void handleEvent(Event* ev) = 0;

private:
	std::thread m_thread;
	EventQueue m_events;

	std::atomic<bool> m_stopThread = false;
};