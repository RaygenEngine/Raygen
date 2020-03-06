#include "test.h"

#include "engine/Object.h"
#

MulticastObjectEvent<int32> testEvent;

struct TestObj : public Object {
	int32 x{ 0 };
	void MemberFunc(int32 v) { x = v; }
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
