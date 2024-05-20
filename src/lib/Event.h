#pragma once

#include <vector>
#include <assert.h>
#include <iostream>

class Event
{
public:
    virtual ~Event()
    {
        assert(freed_through_free_event == true);
        assert(!already_freed);
        // This shouldn't stay valid through a destructor, but it's a "just in case"
        already_freed = true;
    }
    virtual void freeEvent()
    {
        assert(!freed_through_free_event);
        assert(!already_freed);
        freed_through_free_event = true;
        // In practice, this will re-add itself to some free list
        delete this;
    }
    virtual const char *kind() const = 0;

private:
    bool freed_through_free_event = false;
    bool already_freed = false;
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