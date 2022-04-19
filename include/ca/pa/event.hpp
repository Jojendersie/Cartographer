#pragma once

namespace ca { namespace pa {

	/// Observer pattern with O(1) subscribe, unsubscribe and constant memory footprint
	/// of all elements.
	/// This system is designed for synchronous event handling (not thread safe!).
	template < typename EventType >
	class Observer
	{
	public:
		Observer() = default;

		Observer(const Observer& _other)
		{
			// Copy ctor - we need to register the new one on its own.
			if(_other.m_next || _other.m_prev)
			{
				m_next = _other.m_next;
				m_prev = const_cast<Observer*>(&_other);
				_other.m_next = this;
				if(m_next)
					m_next->m_prev = this;
			}
		}

		Observer(Observer&& _other)
		{
			m_next = _other.m_next;
			m_prev = _other.m_prev;
			if(m_next) m_next->m_prev = this;
			if(m_prev) m_prev->m_next = this;
		}

		virtual ~Observer()
		{
			// Remove this element from the double linked list
			if(m_prev) m_prev->m_next = m_next;
			if(m_next) m_next->m_prev = m_prev;
		}

		Observer& operator = (const Observer& _other)
		{
			detach();
			new (this) Observer(_other);
			return *this;
		}

		Observer& operator = (Observer&& _other)
		{
			detach();
			new (this) Observer(_other);
			return *this;
		}

		/// Central interface method to override which reacts to the changes of the Observable.
		virtual void notify(const EventType* ev) = 0;

		/// Remove this element from its current observer list
		void detach()
		{
			// Remove this element from the double linked list
			if(m_prev) m_prev->m_next = m_next;
			if(m_next) m_next->m_prev = m_prev;
		}

	private:
		/// Double linked list of all registered elements.
		mutable Observer* m_next = nullptr;
		mutable Observer* m_prev = nullptr;
		template<typename T>
		friend class Observable;
	};

	/// The class that produces the events.
	template < typename EventType >
	class Observable
	{
	public:
		/// Method must be called from derived class whenever a significant change occurs
		void notify_observers(const EventType* ev)
		{
			Observer<EventType>* next = m_nullObserver.m_next;
			while(next)
			{
				next->notify(ev);
				next = next->m_next;
			}
		}

		/// Register an Observer to be notified if this instance changes.
		void register_observer(Observer<EventType>& _obs)
		{
			_obs.detach(); // Detach from previous list.
			_obs.m_prev = &m_nullObserver;
			_obs.m_next = m_nullObserver.m_next;
			m_nullObserver.m_next = &_obs;
			if(_obs.m_next) _obs.m_next->m_prev = &_obs;
		}

		/// Detach from list. Note that this is only a fancy name for the Observer.detach().
		/// Deletion of objects... will always keep a clean state (even without calling this).
		void unregister_observer(Observer<EventType>& _obs)
		{
			_obs.detach();
		}

	private:
		class NullObserver : public Observer<EventType> { public: void notify(const EventType* ev) override {} };
		NullObserver m_nullObserver;
	};

}}