#pragma once

#include <vector>
#include <assert.h>

class Event
{
public:
    virtual ~Event()
    {
        assert(freed_through_free_event == true);
    }
    void freeEvent()
    {
        // In practice, this will re-add itself to some free list
        freed_through_free_event = true;
        delete this;
    }
    virtual const char *kind() const = 0;

private:
    bool freed_through_free_event = false;
};

class IEventGenerator
{
public:
    virtual Event *nextEvent() = 0;
};

class EventGenerator : public IEventGenerator
{
public:
    Event *nextEvent() override
    {
        if (events.empty())
        {
            return nullptr;
        }
        Event *event = events.back();
        events.pop_back();
        return event;
    }

    bool isEmpty() const
    {
        return events.empty();
    }

    // Takes an event created by new and takes ownership of the event
    void addEvent(Event *e)
    {
        events.push_back(e);
    }

private:
    std::vector<Event *> events;
};