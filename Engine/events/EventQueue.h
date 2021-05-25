#pragma once

#include <mutex>
#include <atomic>
#include <queue>

class Event;

template <typename T>
struct SharedPtrComparator {
    bool operator()(const std::shared_ptr<T>& lhs,
        const std::shared_ptr<T>& rhs) const
    {
        return lhs < rhs;
    }
};

class EventQueue
{
public:
    EventQueue() = default;
    ~EventQueue();
    void addEvent(std::unique_ptr<Event> ev);

    // Will block thread until there is a new event
    std::shared_ptr<Event> nextEvent();

private:
    mutable std::mutex m_eventQueueMutex;
    std::condition_variable m_eventQueueCond;
    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, SharedPtrComparator<Event>> m_events;
    std::atomic_bool m_stopThread;
};