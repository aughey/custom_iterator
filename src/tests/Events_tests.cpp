#include <gtest/gtest.h>
#include "Events.h"

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

// Create a factory for events and free the events manually
TEST(EventTests, BasicAssertions)
{
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    int eventcount = 0;
    for (auto event = events.nextEvent(); event != nullptr; event = events.nextEvent())
    {
        eventcount++;
        event->freeEvent(); // <- This is the challenging statement because sometimes we forget.
    }
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}

TEST(EventTests, LeadTests)
{
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    int eventcount = 0;
    for (auto event = events.nextEvent(); event != nullptr; event = events.nextEvent())
    {
        eventcount++;
        if (std::string("SpecialEvent") == event->kind())
        {
            event->freeEvent(); // <-- this is where things go wrong because there's a branch that doesn't free
        }
    }
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 1);
}

TEST(EventTests, WrappingDeleter)
{
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    int eventcount = 0;
    // Manually wrap every event into a unique_ptr with a customer deleter so that it always works.
    // However, this is a little bit verbose.
    for (auto event = EventDeleter::wrap(events.nextEvent()); event != nullptr; event = EventDeleter::wrap(events.nextEvent()))
    {
        eventcount++;
    }
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}

TEST(EventTests, WhileLoop)
{
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    int eventcount = 0;
    // The while loop is a little bit more concise than the for loop, and I really should stop here.
    while (auto event = EventDeleter::wrap(events.nextEvent()))
    {
        ASSERT_NE(event, nullptr);
        eventcount++;
    }
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}

TEST(EventTests, WrapFactory)
{
    EventFactory events;
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    // WrapFactory is a class that wraps the EventFactory and returns unique_ptr<Event> with a custom deleter
    // automagically without an explicit call to wrap.  It just makes the nextEvent signature method different.
    WrapFactory wrapped_events(events);

    int eventcount = 0;
    // Now our while loop looks even more concise.
    while (auto event = wrapped_events.nextEvent())
    {
        ASSERT_NE(event, nullptr);
        eventcount++;
    }
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}

TEST(EventTests, EventRange)
{
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    int eventcount = 0;
    // WrapFactory also implements the begin and end methods to be used in a range-based for loop.
    for (auto event : WrapFactory(events))
    {
        ASSERT_NE(event, nullptr);
        eventcount++;
    }
    EXPECT_TRUE(events.is_empty());
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}