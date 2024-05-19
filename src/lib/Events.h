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
    // We are constructed with either a raw Event pointer which we take ownership of
    // or a unique_ptr<Event> which we assume ownership of.
    EventIterator(Event *me, WrapFactory &events) : me(me), events(events) {}
    EventIterator(EventDeleter::PointerType me, WrapFactory &events) : me(me.release()), events(events) {}

    // The dereference will provide a unique_ptr<Event> and we give up ownership of that event.
    // This will not allow us to dereference the iterator twice, so is only really useful in a range-based for loop
    // or similar iterator pattern that uses the iterators only once.
    EventDeleter::PointerType operator*()
    {
        return EventDeleter::PointerType(me.release());
    }

    // The prefix increment will get the next event from the WrapFactory and store it in the iterator.
    EventIterator &operator++()
    {
        me = events.nextEvent();
        return *this;
    }

    // This is 99.9% of the time the end of the loop check.  The end iterator will have a nullptr event.
    bool operator!=(const EventIterator &other)
    {
        return other.me != me;
    }

private:
    EventDeleter::PointerType me;
    WrapFactory &events;
};

// Since EventIterator knows about WrapFactory, we have to define the functions outside of the class definition.
// Begin loads the EventIterator with the next event and a reference to ourselves.
EventIterator WrapFactory::begin()
{
    return EventIterator(this->nextEvent(), *this);
}
// End loads the EventIterator with a nullptr event and a reference to ourselves.
EventIterator WrapFactory::end()
{
    return EventIterator(nullptr, *this);
}
