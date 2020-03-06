#pragma once
#include "core/Event.h"
#include <functional>

class Object {
	std::vector<MulticastObjectEventBase*> m_boundEvents;

protected:
	Object() = default;
	~Object()
	{
		for (auto& event : m_boundEvents) {
			event->UnbindNoRemoveFromObject(this);
		}
	};

	template<typename... Args>
	friend struct MulticastObjectEvent;

public:
	Object(const Object&) = delete;
	Object(Object&&) = delete;
	Object& operator=(const Object&) = delete;
	Object& operator=(Object&&) = delete;
};
