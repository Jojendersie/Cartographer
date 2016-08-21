#pragma once

// This unit contains a selfmade shared_ptr - the RefPtr. The advantage of an internal reference
// counter is the ability to create new shared_ptr from a raw pointer.
// This allows for example to register/unregister an object from a function inside the object
// itselft. I.e. the object can create new shared_ptrs from inside using 'this'.
// Another advantage is that there is no pointer-memory overhead. Each RefPtr is only as large
// as a usual pointer.

namespace ca { namespace pa {

	/// Base class for all objects which are used with the pointer.
	class ReferenceCountable
	{
	public:
		virtual ~ReferenceCountable() = default;

	private:
		// RefPtr should be the only class to manage the reference counter.
		template<typename T> friend class RefPtr;
		unsigned m_refCounter = 0;
	};

	/// A pointer which behaves like a shared_ptr.
	template<typename T>
	class RefPtr
	{
	public:
		// Construction and copies
		RefPtr() : m_ptr(nullptr)							{}
		explicit RefPtr(T* _object) : m_ptr(_object)		{ m_ptr->m_refCounter++; }

		template<typename T2, class = typename std::enable_if<std::is_convertible<T2*,T*>::value || std::is_base_of<T2,T>::value, void>::type>
		RefPtr(const RefPtr<T2>& _other)
		{
			if(std::is_convertible<T2*,T*>::value)
				m_ptr = static_cast<T*>(_other.m_ptr);
			else
				m_ptr = dynamic_cast<T*>(_other.m_ptr);
			if(m_ptr)
				m_ptr->m_refCounter++;
		}
		RefPtr(const RefPtr& _other) : m_ptr(_other.m_ptr)
		{
			if(m_ptr)
				m_ptr->m_refCounter++;
		}

		template<typename T2, class = typename std::enable_if<std::is_convertible<T2*,T*>::value || std::is_base_of<T2,T>::value, void>::type>
		RefPtr(RefPtr<T2>&& _other)
		{
			if(std::is_convertible<T2*,T*>::value)
				m_ptr = static_cast<T*>(_other.m_ptr);
			else
				m_ptr = dynamic_cast<T*>(_other.m_ptr);
			if(m_ptr)
				_other.m_ptr = nullptr;
		}
		RefPtr(RefPtr&& _other) : m_ptr(_other.m_ptr)
		{
			if(m_ptr)
				_other.m_ptr = nullptr;
		}

		RefPtr& operator = (const RefPtr& _other)			{ this->~RefPtr(); m_ptr = _other.m_ptr; m_ptr->m_refCounter++; return *this; }
		RefPtr& operator = (T* _other)						{ this->~RefPtr(); m_ptr = _other;       m_ptr->m_refCounter++; return *this; }
		RefPtr& operator = (RefPtr&& _other)				{ this->~RefPtr(); m_ptr = _other.m_ptr; _other.m_ptr = nullptr; return *this; }

		// Deletion
		~RefPtr()
		{
			if(m_ptr && --m_ptr->m_refCounter == 0)
				delete m_ptr;
		}

		// Access
		T& operator * ()				{ return *m_ptr; }
		const T& operator * () const	{ return *m_ptr; }
		T* operator -> ()				{ return m_ptr; }
		const T* operator -> () const	{ return m_ptr; }
		T* get()						{ return m_ptr; }
		const T* get() const			{ return m_ptr; }

		// Check
		explicit operator bool ()				{ return m_ptr != nullptr; }
		bool operator == (const RefPtr& _other)	{ return m_ptr == _other.m_ptr; }
		bool operator != (const RefPtr& _other)	{ return m_ptr != _other.m_ptr; }
		bool operator < (const RefPtr& _other)	{ return m_ptr < _other.m_ptr; }
		bool operator <= (const RefPtr& _other)	{ return m_ptr <= _other.m_ptr; }
		bool operator >= (const RefPtr& _other)	{ return m_ptr >= _other.m_ptr; }
		bool operator > (const RefPtr& _other)	{ return m_ptr > _other.m_ptr; }

	private:
		T* m_ptr;

		template<typename T2> friend class RefPtr;
	};

}} // namespace ca::pa