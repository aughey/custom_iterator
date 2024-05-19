#include <gtest/gtest.h>
#include "Events.h"

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
        event->freeEvent();
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
            event->freeEvent();
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
    auto events = EventFactory();
    int freecount = 0;
    events.addEvent(new SpecialEvent(freecount));
    events.addEvent(new AnotherEvent(freecount));

    auto wrapped_events = WrapFactory(events);

    int eventcount = 0;
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
    for (auto event : WrapFactory(events))
    {
        ASSERT_NE(event, nullptr);
        eventcount++;
    }
    EXPECT_TRUE(events.is_empty());
    EXPECT_EQ(eventcount, 2);
    EXPECT_EQ(freecount, 0);
}