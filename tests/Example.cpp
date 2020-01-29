#include "test.h"

// Include the engine headers required for testing
#include "system/EngineEvents.h"
// Include any Raygen-Test specific header files (if any)


TEST_CASE("Multicast Event & Listener")
{
	MulticastEvent<int32> SimpleEvent;

	REQUIRE(SimpleEvent.IsBound() == false);
	{
		MulticastEvent<int32>::Listener Listener{ SimpleEvent };
		Listener.Bind([](int32 value) { REQUIRE(value == 42); });

		REQUIRE(SimpleEvent.IsBound() == true);

		SimpleEvent.Broadcast(42);
	}
	REQUIRE(SimpleEvent.IsBound() == false);
}

struct EventListenerTest {
	DECLARE_EVENT_LISTENER(focusListener, Event::OnWindowFocus);

	void OnWindowFocus(bool newFocus) { REQUIRE(newFocus == true); }

	EventListenerTest() { focusListener.BindMember(this, &EventListenerTest::OnWindowFocus); }
};

TEST_CASE("Engine Event & Scoped Listener")
{
	Event::OnWindowFocus.Broadcast(false);
	{
		EventListenerTest listener;
		Event::OnWindowFocus.Broadcast(true);
		REQUIRE(Event::OnWindowFocus.IsBound() == true);
	}
	REQUIRE(Event::OnWindowFocus.IsBound() == false);
}
