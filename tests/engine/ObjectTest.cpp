#include "test.h"

#include "engine/Events.h"


MulticastEvent<int32> testEvent;

struct TestObj : public Object {
	int32 x{ 0 };
	void MemberFunc(int32 v) { x = v; }
	void MemberFunc2(int32 v) { x = -1; }
};


TEST("Object Event test")
{
	TestObj* obj = new TestObj();

	int32 variable = 0;
	auto l = [&variable](int32 param) {
		variable = param;
	};

	testEvent.Bind(obj, l);
	testEvent.Broadcast(3);
	REQ(variable == 3);


	testEvent.Unbind(obj);
	testEvent.Broadcast(5);
	REQ(variable != 5);


	testEvent.Bind(obj, &TestObj::MemberFunc);
	testEvent.Broadcast(1);
	REQ(obj->x == 1);

	delete obj;

	REQ(testEvent.IsBound() == false);
}


TEST("Out of scope capture regression test")
{
	TestObj* obj = new TestObj();
	TestObj* obj2 = new TestObj();

	{

		int32 variable = 0;
		auto l = [&variable](int32 param) {
			variable = param;
		};

		testEvent.Bind(obj, l);
		testEvent.Broadcast(3);
		REQ(variable == 3);


		testEvent.Unbind(obj);
		testEvent.Broadcast(5);
		REQ(variable != 5);

		testEvent.Bind(obj, &TestObj::MemberFunc);
	}

	testEvent.Bind(obj2, &TestObj::MemberFunc2);
	testEvent.Broadcast(1);
	REQ(obj->x == 1);
	REQ(obj2->x == -1);

	delete obj;
	delete obj2;

	REQ(testEvent.IsBound() == false);
}
