#pragma once

#include <memory>
#include "Event.h"

namespace wrapper
{
    /// @brief unique_ptr deleter for events that require freeEvent to be called when done.
    class EventDeleter
    {
    public:
        using PointerType = std::shared_ptr<Event>;
        /// @brief unique_ptr interface to drop an event
        /// @param e the event to be dropped
        inline void operator()(Event *e)
        {
            if (e)
            {
                e->freeEvent();
            }
        }
        static inline PointerType wrap(Event *e)
        {
            return PointerType(e, EventDeleter());
        }
    };

    /// @brief object created on the stack to generate smart pointer versions of Events through the nextEvent convention.
    ///
    /// nextEvent in IEventGenerator returns a raw Event pointer that requires the user to free explicilty.
    /// This class implements the same calling mechanism of nextEvent, but return a unique_ptr version that
    /// will always free correctly.
    class WrappedEvents
    {
    public:
        /// @brief Takes a reference to an IEventGenerator.  Does not take ownership.
        /// @param events
        WrappedEvents(IEventGenerator &events) : events(events) {}
        // Create a nextEvent that "look like" the IEventGenerator nextEvent, but actually
        // return a smart pointer version of our Event object.
        inline EventDeleter::PointerType nextEvent()
        {
            return EventDeleter::wrap(events.nextEvent());
        }

    private:
        /// @brief Hidden iterator for events.  Hidden here because user will never create a version of this.
        /// Implements the iterator "trait" for going through events.
        class EventIterator
        {
        public:
            // Takes ownership of the passed in me pointer type
            EventIterator(const EventDeleter::PointerType &me, WrappedEvents &events) : me(me), events(events) {}

            // Relinquish control of the event to the caller through smart pointers
            inline EventDeleter::PointerType operator*()
            {
                // Will happy make a copy of it
                return me;
            }

            /// @brief Get the next event through the events reference we are holding.
            /// @return ourself that has been advanced to the next event.
            inline EventIterator &operator++()
            {
                this->me = events.nextEvent();
                return *this;
            }

            /// @brief Compare ourselves to some other iterator for inquality
            /// @param other other iterator that we are comparing against
            /// @return true if the other is not equal to oursevles
            inline bool operator!=(const EventIterator &other)
            {
                return other.me != me;
            }

        private:
            EventDeleter::PointerType me;
            WrappedEvents &events;
        };

        IEventGenerator &events;

    public:
        /// @brief Generate an iterator pointing to the next event that can be gotten.
        /// @return An iterator
        inline EventIterator begin()
        {
            return EventIterator(this->nextEvent(), *this);
        }
        /// @brief Generator an iterator pointing to the end.  This is indicated by a null event
        /// @return "last" event (null)
        inline EventIterator end()
        {
            return EventIterator(EventDeleter::PointerType(nullptr), *this);
        }
    };

}