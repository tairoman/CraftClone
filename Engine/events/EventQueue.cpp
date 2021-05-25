#include "EventQueue.h"
#include "Event.h"

EventQueue::~EventQueue()
{
    m_stopThread = true;
    m_eventQueueCond.notify_all();
}

void EventQueue::addEvent(std::unique_ptr<Event> ev)
{
	std::unique_lock<std::mutex> lck(m_eventQueueMutex);
	m_events.push(std::move(ev));
}

std::shared_ptr<Event> EventQueue::nextEvent()
{
    std::unique_lock<std::mutex> lck(m_eventQueueMutex);
    m_eventQueueCond.wait(lck, [this]() {
        return !m_events.empty() || m_stopThread.load();
        });
    if (m_stopThread.load()) {
        return std::make_shared<Event>();
    }
    const auto event = m_events.top();
    m_events.pop();
    return event;
}

