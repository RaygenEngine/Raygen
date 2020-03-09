#pragma once

#include "engine/Logger.h"
#include "engine/Object.h"

#include <algorithm>
#include <functional>
#include <vector>


struct MulticastObjectEventBase {
protected:
	friend class Object;
	virtual void UnbindFromDestructor(Object* obj) = 0;

public:
	virtual void Unbind(Object* obj) = 0;
};

template<class... Args>
struct MulticastEvent : public MulticastObjectEventBase {
	using FunctionType = std::function<void(Args...)>;
	friend class Object;

protected:
	std::unordered_map<Object*, FunctionType> m_binds;

	void UnbindFromDestructor(Object* obj) override { m_binds.erase(obj); }

public:
	void Unbind(Object* obj) override
	{
		m_binds.erase(obj);
		obj->m_boundEvents.erase(std::remove_if(
			obj->m_boundEvents.begin(), obj->m_boundEvents.end(), [this](auto other) { return other == this; }));
	}


	void Bind(Object* obj, FunctionType&& f)
	{
		CLOG_ABORT(m_binds.count(obj), "Binding an object that is already bound.");

		obj->m_boundEvents.push_back(this);
		m_binds.emplace(obj, f);
	}

	template<typename T>
	void Bind(T* obj, void (T::*f)(Args...))
	{
		Bind(obj, [f, obj](Args... args) { (obj->*f)(args...); });
	}


	void Broadcast(Args... args)
	{
		for (auto& [ptr, bind] : m_binds) {
			bind(args...);
		}
	}

	[[nodiscard]] constexpr bool IsBound() const noexcept { return m_binds.size() > 0; }
};
