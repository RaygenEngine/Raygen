#pragma once
#include <functional>
#include <vector>
#include <algorithm>

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

		[[nodiscard]] bool IsBound() const { return m_registered; }

		~Listener() { Unbind(); }
	};

private:
	std::vector<Listener*> listeners;

	void Bind(Listener* listener) { listeners.push_back(listener); }

	void Unbind(Listener* listener) { listeners.erase(std::remove(listeners.begin(), listeners.end(), listener)); }

public:
	void Broadcast(Args... args)
	{
		for (auto listener : listeners) {
			listener->m_callback(std::forward<Args>(args)...);
		}
	}
};
