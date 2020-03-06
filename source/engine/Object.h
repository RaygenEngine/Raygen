#pragma once

#include "reflection/TypeId.h"

#include <functional>

struct MulticastObjectEventBase;
class Object {
	std::vector<MulticastObjectEventBase*> m_boundEvents;

protected:
	Object() = default;
	~Object();

	template<typename... Args>
	friend struct MulticastEvent;

public:
	Object(const Object&) = delete;
	Object(Object&&) = delete;
	Object& operator=(const Object&) = delete;
	Object& operator=(Object&&) = delete;
};
