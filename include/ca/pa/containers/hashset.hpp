#pragma once

#include <cinttypes>
#include <type_traits>
#include <cstring>

namespace ca { namespace pa {

template<typename T, typename Hash = std::hash<T>, typename Compare = std::equal_to<T>>
class HashSet
{
public:
	/// Handles are direct accesses into a specific hashmap.
	/// Any add or remove in the HM will invalidate the handle without notification.
	/// A handle might be usable afterwards, but there is no guaranty.
	template<typename SetT, typename DataT>
	class HandleT
	{
		SetT* map;
		uint32_t idx;

		HandleT(SetT* _map, uint32_t _idx) :
			map(_map),
			idx(_idx)
		{}

		friend HashSet;
	public:
		DataT& value() const { return map->m_keys[idx].value; }

		operator bool () const { return map != nullptr; }

		HandleT& operator ++ ()
		{
			++idx;
			// Move forward while the element is empty.
			while((idx < map->m_capacity) && (map->m_keys[idx].dist == 0xffffffff))
				++idx;
			// Set to invalid handle?
			if(idx >= map->m_capacity) { idx = 0; map = nullptr; }
			return *this;
		}

		bool operator == (const HandleT& _other) const { return map == _other.map && idx == _other.idx; }
		bool operator != (const HandleT& _other) const { return map != _other.map || idx != _other.idx; }

		// The dereference operator has no function other than making this handle compatible
		// for range based loops.
		DataT& operator * () const { return map->m_keys[idx].value; }
	};

	typedef HandleT<HashSet, T> Handle;
	typedef HandleT<const HashSet, const T> ConstHandle;



	explicit HashSet(uint32_t _expectedElementCount = 15) :
		m_capacity(estimateCapacity(_expectedElementCount)),
		m_size(0)
	{
		m_keys = static_cast<Key*>(malloc(sizeof(Key) * m_capacity));
		
		for(uint32_t i = 0; i < m_capacity; ++i)
			m_keys[i].dist = 0xffffffff;
	}

	HashSet(HashSet&& _other) :
		m_capacity(_other.m_capacity),
		m_size(_other.m_size),
		m_keys(_other.m_keys)
	{
		_other.m_keys = nullptr;
	}

	HashSet& operator = (HashSet&& _rhs)
	{
		this->~HashSet();
		m_capacity = _rhs.m_capacity;
		m_size = _rhs.m_size;
		m_keys = _rhs.m_keys;
		_rhs.m_keys = nullptr;
		return *this;
	}

	~HashSet()
	{
		if(m_keys)
		{
			// Call destructors of allocated keys and data
			for(uint32_t i = 0; i < m_capacity; ++i)
				if(m_keys[i].dist != 0xffffffff)
				{
					m_keys[i].value.~T();
				}
		}
		free(m_keys);
	}

	void add(T _value)
	{
		using namespace std;
		uint32_t h = (uint32_t)m_hash(_value);//hash(reinterpret_cast<const uint32_t*>(&_key), sizeof(_key) / 4);
	restartAdd:
		uint32_t d = 0;
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff) // while not empty cell
		{
			if(m_keyCompare(m_keys[idx].value, _value)) // return if keys are identically
			{
				return;
			}
			// probing (collision)
			// Since we have encountered a collision: should we resize?
			if(m_size > 0.77 * m_capacity) {
				reserve(m_size * 2);
				// The resize changed everything beginning from the index
				// to the content of the target cell. Restart the search.
				goto restartAdd;
			}

			if(m_keys[idx].dist < d) // Swap and then insert the element from this location instead
			{
				swap(_value, m_keys[idx].value);
				swap(d, m_keys[idx].dist);
			}
			++d;
		//	idx = (idx + 1) % m_capacity;
			if(++idx >= m_capacity) idx = 0;
		}
		new (&m_keys[idx].value)(T)(move(_value));
		m_keys[idx].dist = d;
		++m_size;
	}

	// Remove an element if it exists
	void remove(const T& _value)
	{
		remove(find(_value));
	}

	// Remove an existing element
	void remove(const Handle& _element)
	{
		using namespace std;
		if(_element)
		{
			m_keys[_element.idx].dist = 0xffffffff;
			m_keys[_element.idx].value.~T();
			--m_size;
			uint32_t i = _element.idx;
			uint32_t next = (_element.idx + 1) % m_capacity;
			while((m_keys[next].dist != 0) && (m_keys[next].dist != 0xffffffff))
			{
				m_keys[i].value = move(m_keys[next].value);
				m_keys[i].dist = m_keys[next].dist - 1;
				i = next;
				if(++next >= m_capacity) next = 0;
			}
		}
	}

	Handle find(const T& _value)
	{
		uint32_t d = 0;
		uint32_t h = (uint32_t)m_hash(_value);//hash(reinterpret_cast<const uint32_t*>(&_key), sizeof(_key) / 4);
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff && d <= m_keys[idx].dist)
		{
			if(m_keyCompare(m_keys[idx].value, _value))
				return Handle(this, idx);
			if(++idx >= m_capacity) idx = 0;
			++d;
		}
		return Handle(nullptr, 0);
	}

	bool contains(const T& _value) const
	{
		return const_cast<HashSet*>(this)->find(_value);
	}

	// Change the capacity if possible. It cannot be decreased below 'size'.
	void resize(uint32_t _newCapacity)
	{
		using namespace std;
		//if(_newCapacity == m_capacity) return;
		if(_newCapacity < m_size) _newCapacity = m_size;

		HashSet tmp(_newCapacity);
		// Find all data sets and readd them to the new temporary hm
		for(uint32_t i = 0; i < m_capacity; ++i)
		{
			if(m_keys[i].dist != 0xffffffff)
			{
				tmp.add(move(m_keys[i].value));
				m_keys[i].dist = 0xffffffff;
			}
		}

		// Use the temporary map now and let the old memory be destroyed.
		swap(*this, tmp);
	}

	void reserve(uint32_t _exptectedElementCount)
	{
		resize(estimateCapacity(_exptectedElementCount));
	}

	/// Remove all elements from the set but keep the capacity.
	void clear()
	{
		if(m_size > 0)
		{
			for(uint32_t i = 0; i < m_capacity; ++i)
				if(m_keys[i].dist != 0xffffffff)
				{
					m_keys[i].value.~T();
					m_keys[i].dist = 0xffffffff;
				}
			m_size = 0;
		}
	}

	uint32_t size() const { return m_size; }

	/// Returns the first element found in the map or an invalid handle when the map is empty.
	Handle begin()
	{
		if(m_size == 0)
			return Handle(nullptr, 0);

		for(uint32_t i = 0; i < m_capacity; ++i)
			if(m_keys[i].dist != 0xffffffff)
				return Handle(this, i);

		return Handle(nullptr, 0);
	}
	ConstHandle begin() const
	{
		// COPY of begin() <noconst>
		if(m_size == 0)
			return ConstHandle(nullptr, 0);

		for(uint32_t i = 0; i < m_capacity; ++i)
			if(m_keys[i].dist != 0xffffffff)
				return ConstHandle(this, i);

		return ConstHandle(nullptr, 0);
	}

	/// Return the invalid handle for range based for loops
	Handle end()
	{
		return Handle(nullptr, 0);
	}
	ConstHandle end() const
	{
		return ConstHandle(nullptr, 0);
	}

private:
	uint32_t m_capacity;
	uint32_t m_size;

	struct Key
	{
		T value;
		uint32_t dist; // robin hood cashing offset
	};

	Key* m_keys;
	Hash m_hash;
	Compare m_keyCompare;
	
	static uint32_t estimateCapacity(uint32_t _exptectedElementCount)
	{
		 // Try to keep the capacity odd (prime would be even better)
		return (uint32_t(_exptectedElementCount * 1.3) | 1) + 2;
	}

	/*uint32_t hash(const uint32_t* _key, unsigned _numWords)
	{
		// TODO: general purpose hash function
		return *_key;
	}*/
};

}} // namespace ca::pa