#include <gtest/gtest.h>
#include <string>
#include "EventWrapper.h"
#include "Event.h"

using namespace wrapper;

class MyEvent : public Event
{
public:
    MyEvent(int &freecount) : freecount(freecount)
    {
        freecount++;
    }
    virtual ~MyEvent()
    {
        freecount--;
    }
    const char *kind() const override
    {
        return "MyEvent";
    }

private:
    int &freecount;
};

class AnotherEvent : public Event
{
public:
    AnotherEvent(int &freecount) : freecount(freecount)
    {
        freecount++;
    }
    virtual ~AnotherEvent()
    {
        freecount--;
    }
    const char *kind() const override
    {
        return "AnotherEvent";
    }

private:
    int &freecount;
};

TEST(EventTests, Create)
{
    EventGenerator events;
    ASSERT_TRUE(events.isEmpty());
}

TEST(EventTests, BasicUsage)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    ASSERT_FALSE(events.isEmpty());
    ASSERT_EQ(live_events, 1);

    auto event = events.nextEvent();
    ASSERT_EQ(std::string("MyEvent"), event->kind());
    ASSERT_EQ(live_events, 1);
    event->freeEvent();
    ASSERT_EQ(live_events, 0);

    ASSERT_TRUE(events.isEmpty());
    ASSERT_EQ(live_events, 0);
}

TEST(EventTests, ProblemEmbodied)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    events.addEvent(new MyEvent(live_events));
    events.addEvent(new AnotherEvent(live_events));
    events.addEvent(new MyEvent(live_events));
    ASSERT_EQ(live_events, 4);

    // Typical user code
    while (auto event = events.nextEvent())
    {
        if (std::string("MyEvent") == event->kind())
        {
            event->freeEvent();
        }
    }

    // We have leaked memory because AnotherEvent isn't freed
    ASSERT_EQ(live_events, 1);
}

TEST(EventTests, RawImplement)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    events.addEvent(new AnotherEvent(live_events));
    ASSERT_EQ(live_events, 2);

    // Typical user code
    while (Event *event_ptr = events.nextEvent())
    {
        std::unique_ptr<Event, EventDeleter> event(event_ptr);

        if (std::string("MyEvent") == event->kind())
        {
            // Do something special with event
        }
    }

    // We have leaked memory because AnotherEvent isn't freed
    ASSERT_EQ(live_events, 0);
}

TEST(EventTests, WrapperImplement)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    events.addEvent(new AnotherEvent(live_events));
    ASSERT_EQ(live_events, 2);

    // Temp adapter to give nextEvent a different interface
    WrappedEvents wrapped(events);

    // Typical user code
    while (auto event = wrapped.nextEvent())
    {
        if (std::string("MyEvent") == event->kind())
        {
            // Do something special with event
        }
    }

    // We have leaked memory because AnotherEvent isn't freed
    ASSERT_EQ(live_events, 0);
}

TEST(EventTests, RangeBasedForLooping)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    events.addEvent(new AnotherEvent(live_events));
    ASSERT_EQ(live_events, 2);

    // Typical user code
    for (auto event : WrappedEvents(events))
    {
        if (std::string("MyEvent") == event->kind())
        {
            // Do something special with event
        }
    }

    // We have leaked memory because AnotherEvent isn't freed
    ASSERT_EQ(live_events, 0);
}

TEST(EventTests, DoubleFree)
{
    EventGenerator events;

    int live_events = 0;

    events.addEvent(new MyEvent(live_events));
    events.addEvent(new AnotherEvent(live_events));
    ASSERT_EQ(live_events, 2);

    // Typical user code
    for (auto event : WrappedEvents(events))
    {
        if (std::string("MyEvent") == event->kind())
        {
            // Need to filter out in CI that there are no user calls to freeEvent
            // event->freeEvent(); /// BADD
        }
    }

    // We have leaked memory because AnotherEvent isn't freed
    ASSERT_EQ(live_events, 0);
}