#include "EventThread.h"

EventThread::EventThread()
{
	m_thread = std::thread([this]() {
		onStart();

		while (!m_stopThread.load()) {

			// Will block until new event on queue
			auto event = m_events.nextEvent();
			handleEvent(event.get());
		}

		onFinish();
	});
}

void EventThread::stop()
{
	m_stopThread = true;
}

void EventThread::join()
{
	m_thread.join();
}

void EventThread::pushEvent(std::unique_ptr<Event> ev)
{
	m_events.addEvent(std::move(ev));
}
