#include "test.h"

// Include the engine headers required for testing
#include "engine/EngineEvents.h"
// Include any Raygen-Test specific header files (if any)


TEST("Multicast Event & Listener")
{
	MulticastEvent<int32> SimpleEvent;

	REQ(SimpleEvent.IsBound() == false);
	{
		MulticastEvent<int32>::Listener Listener{ SimpleEvent };
		Listener.Bind([](int32 value) { REQ(value == 42); });

		REQ(SimpleEvent.IsBound() == true);

		SimpleEvent.Broadcast(42);
	}
	REQ(SimpleEvent.IsBound() == false);
}

struct EventListenerTest {
	DECLARE_EVENT_LISTENER(focusListener, Event::OnWindowFocus);

	void OnWindowFocus(bool newFocus) { REQUIRE(newFocus == true); }

	EventListenerTest() { focusListener.BindMember(this, &EventListenerTest::OnWindowFocus); }
};

TEST("Engine Event & Scoped Listener")
{
	Event::OnWindowFocus.Broadcast(false);
	{
		EventListenerTest listener;
		Event::OnWindowFocus.Broadcast(true);
		REQ(Event::OnWindowFocus.IsBound() == true);
	}
	REQ(Event::OnWindowFocus.IsBound() == false);
}
