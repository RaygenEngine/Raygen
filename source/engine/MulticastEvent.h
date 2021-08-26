#pragma once
#include "engine/Listener.h"
#include "engine/Logger.h"

#include <algorithm>
#include <functional>
#include <vector>


struct MulticastObjectEventBase {
protected:
	friend class Listener;
	virtual void UnbindFromDestructor(Listener* obj) = 0;

public:
	virtual void Unbind(Listener* obj) = 0;
};

template<class... Args>
struct MulticastEvent : public MulticastObjectEventBase {
	using FunctionType = std::function<void(Args...)>;
	friend class Listener;

protected:
	std::unordered_map<Listener*, FunctionType> m_binds;

	void UnbindFromDestructor(Listener* obj) override { m_binds.erase(obj); }

public:
	void Unbind(Listener* obj) override
	{
		m_binds.erase(obj);
		obj->m_boundEvents.erase(std::remove_if(
			obj->m_boundEvents.begin(), obj->m_boundEvents.end(), [this](auto other) { return other == this; }));
	}


	void Bind(Listener* obj, FunctionType&& f)
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

	// Utility to bind member bool flags.
	// Only bind members of obj as the flag otherwise you invoke UB.
	void BindFlag(Listener* obj, BoolFlag& inFlag)
	{
		Bind(obj, [flag = &inFlag](Args... args) { flag->Set(); });
	}

	void Broadcast(Args... args)
	{
		for (auto& [ptr, bind] : m_binds) {
			bind(args...);
		}
	}

	[[nodiscard]] constexpr bool IsBound() const noexcept { return m_binds.size() > 0; }
};
