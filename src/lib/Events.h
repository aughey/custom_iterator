#pragma once

#include <vector>
#include <memory>

class Event
{
public:
    virtual ~Event() {}
    void freeEvent()
    {
        delete this;
    }
    virtual const char *kind() const = 0;
};

// This class is used to delete events through an unique_ptr customer deleter
class EventDeleter
{
public:
    using PointerType = std::unique_ptr<Event, EventDeleter>;
    void operator()(Event *event)
    {
        event->freeEvent();
    }
    static std::unique_ptr<Event, EventDeleter>
    wrap(Event *event)
    {
        return std::unique_ptr<Event, EventDeleter>(event);
    }
};

class SpecialEvent : public Event
{
public:
    SpecialEvent(int &dec_on_free) : dec_on_free(dec_on_free)
    {
        dec_on_free++;
    }
    virtual ~SpecialEvent()
    {
        dec_on_free--;
    }
    const char *kind() const override
    {
        return "SpecialEvent";
    }

private:
    int &dec_on_free;
};

class AnotherEvent : public Event
{
public:
    AnotherEvent(int &dec_on_free) : dec_on_free(dec_on_free)
    {
        dec_on_free++;
    }
    virtual ~AnotherEvent()
    {
        dec_on_free--;
    }
    const char *kind() const override
    {
        return "AnotherEvent";
    }

private:
    int &dec_on_free;
};

class EventFactory
{
public:
    Event *nextEvent()
    {
        if (events.empty())
        {
            return nullptr;
        }
        Event *event = events.back();
        events.pop_back();
        return event;
    }
    void addEvent(Event *event)
    {
        events.push_back(event);
    }

    bool is_empty() const
    {
        return events.empty();
    }

private:
    std::vector<Event *> events;
};

class EventIterator;

class WrapFactory
{
public:
    // Takes a non-owning reference to EventFactory
    WrapFactory(EventFactory &events) : events(events) {}
    EventDeleter::PointerType nextEvent()
    {
        return EventDeleter::wrap(events.nextEvent());
    }
    EventIterator begin();
    EventIterator end();

private:
    EventFactory &events;
};

// Range based iterator for events that hold a reference to the WrapFactory
class EventIterator
{
public:
    EventIterator(Event *me, WrapFactory &events) : me(me), events(events) {}
    EventIterator(EventDeleter::PointerType me, WrapFactory &events) : me(me.release()), events(events) {}
    EventDeleter::PointerType operator*()
    {
        return EventDeleter::PointerType(me.release());
    }
    EventIterator &operator++()
    {
        me = events.nextEvent();
        return *this;
    }
    bool operator!=(const EventIterator &other)
    {
        return other.me != me;
    }

private:
    EventDeleter::PointerType me;
    WrapFactory &events;
};

EventIterator WrapFactory::begin()
{
    return EventIterator(this->nextEvent(), *this);
}
EventIterator WrapFactory::end()
{
    return EventIterator(nullptr, *this);
}
