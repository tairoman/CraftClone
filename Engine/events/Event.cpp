#include "Event.h"

Event::Event(std::size_t priority) : m_priority(priority) {}

Event::~Event() {}

std::size_t Event::priority() const { return m_priority; }

bool Event::operator<(const Event& b) const
{
    return m_priority < b.priority();
}