#pragma once
#include "reflection/TypeId.h"

#include <functional>

struct MulticastObjectEventBase;
class Listener {
	std::vector<MulticastObjectEventBase*> m_boundEvents;

protected:
	Listener() = default;
	~Listener();

	template<typename... Args>
	friend struct MulticastEvent;

public:
	Listener(const Listener&) = delete;
	Listener(Listener&&) = delete;
	Listener& operator=(const Listener&) = delete;
	Listener& operator=(Listener&&) = delete;
};
