#pragma once

#include <limits>

class Event
{
public:
    Event() = default;
    explicit Event(std::size_t priority);
    virtual ~Event();

    std::size_t priority() const;

    bool operator<(const Event& b) const;
private:
    std::size_t m_priority = std::numeric_limits<std::size_t>::max();
};