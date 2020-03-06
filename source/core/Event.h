#pragma once

#include <algorithm>
#include <functional>
#include <vector>
#include "engine/Logger.h"

template<class... Args>
struct MulticastEvent {
	using FunctionType = std::function<void(Args...)>;

	// Declare this as a member in your class to be able attach to the specific event.
	// This is an autoclean listener which means that when it goes out of scope it automatically removes its
	// registration. As such you can always capture [this](){} in your lambdas, never unbind and always be sure "this"
	// is not a dangling reference
	struct Listener {
	private:
		friend MulticastEvent;
		bool m_registered{ false };
		FunctionType m_callback;
		MulticastEvent& m_event;

	public:
		Listener(MulticastEvent& event)
			: m_event(event)
		{
		}

		// Bind a lambda here that will run when the event gets broadcasted.
		void Bind(FunctionType callback)
		{
			if (!m_registered) {
				m_registered = true;
				m_event.Bind(this);
			}
			m_callback = callback;
		}

		template<typename T>
		using MemberFunc = void (T::*)(Args...);

		// First parameter is the objInstance to call the function for. (Usually 'this')
		// Second is the member function
		// eg call: BindMember(this, &CameraNode::OnResize);
		template<typename T>
		void BindMember(T* objInstance, MemberFunc<T> callback)
		{
			Bind([=](Args... args) { std::invoke(callback, objInstance, args...); });
		}

		void Unbind()
		{
			if (m_registered) {
				m_event.Unbind(this);
				m_registered = false;
			}
		}

		[[nodiscard]] bool IsBound() const noexcept { return m_registered; }

		~Listener() { Unbind(); }
	};

private:
	std::vector<Listener*> listeners;
	std::vector<FunctionType> freeFunctions;

	void Bind(Listener* listener) { listeners.push_back(listener); }

	void Unbind(Listener* listener) { listeners.erase(std::remove(listeners.begin(), listeners.end(), listener)); }

public:
	void Broadcast(Args... args)
	{
		for (auto listener : listeners) {
			listener->m_callback(args...);
		}

		for (auto& func : freeFunctions) {
			func(args...);
		}
	}

	void UnbindObject() {}

	void BindFreeFunction(std::function<void(Args...)>&& func) { freeFunctions.emplace_back(func); }
	void UnbindAllFreeFunctions() noexcept { freeFunctions.clear(); }

	[[nodiscard]] constexpr bool IsBound() const noexcept { return listeners.size() > 0; }
};


class Object;
struct MulticastObjectEventBase {
protected:
	friend class Object;
	std::unordered_map<Object*, size_t> m_objectBindIndex;

	void UnbindNoRemoveFromObject(Object* obj)
	{
		if (auto f = m_objectBindIndex.find(obj); f != m_objectBindIndex.end()) {
			UnbindIndexNoRemoveFromObject(f->second);
			m_objectBindIndex.erase(f);
		}
	}

	virtual void UnbindIndex(size_t index, Object* obj) = 0;
	virtual void UnbindIndexNoRemoveFromObject(size_t index) = 0;


public:
	void Unbind(Object* obj)
	{
		if (auto f = m_objectBindIndex.find(obj); f != m_objectBindIndex.end()) {
			UnbindIndex(f->second, obj);
			m_objectBindIndex.erase(f);
		}
	}
};

template<class... Args>
struct MulticastObjectEvent : public MulticastObjectEventBase {
	using FunctionType = std::function<void(Args...)>;

protected:
	friend class Object;

	std::vector<FunctionType> m_binds;

	void UnbindIndex(size_t index, Object* obj) override
	{
		std::iter_swap(m_binds.end() - 1, m_binds.begin() + index);
		m_binds.erase(m_binds.end() - 1);
		obj->m_boundEvents.erase(std::remove_if(
			obj->m_boundEvents.begin(), obj->m_boundEvents.end(), [this](auto other) { return other == this; }));
	}

	void UnbindIndexNoRemoveFromObject(size_t index) override
	{
		std::iter_swap(m_binds.end() - 1, m_binds.begin() + index);
		m_binds.erase(m_binds.end() - 1);
	}


public:
	void Bind(Object* obj, FunctionType&& f)
	{
		CLOG_ABORT(m_objectBindIndex.count(obj), "Binding an object that is already bound.");

		obj->m_boundEvents.push_back(this);

		size_t pos = m_binds.size();
		m_binds.emplace_back(f);
		m_objectBindIndex.emplace(obj, pos);
	}

	template<typename T>
	void Bind(T* obj, void (T::*f)(Args...))
	{
		Bind(obj, [&f, obj](Args... args) { (obj->*f)(args...); });
	}


	void Broadcast(Args... args)
	{
		for (auto& bind : m_binds) {
			bind(args...);
		}
	}

	[[nodiscard]] constexpr bool IsBound() const noexcept { return m_binds.size() > 0; }
};
