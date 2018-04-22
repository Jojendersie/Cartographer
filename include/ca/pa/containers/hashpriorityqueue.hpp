#pragma once

#include <cinttypes>
#include <type_traits>
#include <cassert>

namespace ca { namespace pa {

	/// Min-heap implementation of a priority queue combined with a hash set to find the
	/// elements fast.
	/// The hash map solves collisions using Robinhood hashing:
	///		- https://cs.uwaterloo.ca/research/tr/1986/CS-86-14.pdf
	///		- http://codecapsule.com/2013/11/11/robin-hood-hashing/
	///		- https://en.wikipedia.org/wiki/Hash_table#Robin_Hood_hashing
	///		- https://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/
	/// \tparam DataT Type of the data to be stored. Must support the operator less <
	///		and a hash functor and an equal comparison.
	template<typename DataT, typename Hash = std::hash<DataT>, typename Compare = std::equal_to<DataT>>
	class HashPriorityQueue
	{
	public:
		/// Range loop iterable handle. Handles are guaranteed to be valid over the
		/// entire lifetime of a datum.
		template<typename HeapT, typename _DataT>
		class HandleT
		{
			HeapT* heap;
			uint32_t dataIdx;

			HandleT(HeapT* _heap, uint32_t _dataIdx) :
				heap(_heap),
				dataIdx(_dataIdx)
			{}

			friend HashPriorityQueue;
		public:
			HandleT() : heap(nullptr), dataIdx(0) {}
			operator bool () const { return heap != nullptr; }
			bool operator == (const HandleT& _other) const { return heap == _other.heap && dataIdx == _other.dataIdx; }
			bool operator != (const HandleT& _other) const { return heap != _other.heap || dataIdx != _other.dataIdx; }
			
			HandleT& operator ++ ()
			{
				uint32_t i = heap->m_data[dataIdx].heapIdx + 1;
				// Set to invalid handle?
				if(i >= heap->m_size) { dataIdx = 0; heap = nullptr; }
				else dataIdx = heap->m_heap[i];
				return *this;
			}

			_DataT& operator * () const { return heap->m_data[dataIdx].data; }
			_DataT* operator -> () const { return &heap->m_data[dataIdx].data; }
		};

		typedef HandleT<HashPriorityQueue, DataT> Handle;
		typedef HandleT<const HashPriorityQueue, const DataT> ConstHandle;

		explicit HashPriorityQueue(uint32_t _reserveSize = 32) :
			m_capacity(_reserveSize),
			m_size(0)
		{
			m_data = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			m_heap = (uint32_t*)malloc(sizeof(uint32_t) * _reserveSize);
			// Mark fields as empty in the hash map
			for(uint32_t i = 0; i < m_capacity; ++i)
				m_data[i].dist = ~0;
		}

		HashPriorityQueue(HashPriorityQueue&& _other) :
			m_capacity(_other.m_capacity),
			m_size(_other.m_size),
			m_data(_other.m_data),
			m_heap(_other.m_heap)
		{
			_other.m_data = nullptr;
			_other.m_heap = nullptr;
		}

		HashPriorityQueue& operator = (HashPriorityQueue&& _rhs)
		{
			this->~PriorityQueue();
			m_capacity = _rhs.m_capacity;
			m_size = _rhs.m_size;
			m_data = _rhs.m_data;
			m_heap = _rhs.m_heap;
			_rhs.m_data = nullptr;
			_rhs.m_heap = nullptr;
			return *this;
		}


		~HashPriorityQueue()
		{
			if(m_data)
			{
				clear();
				free(m_data);
				free(m_heap);
			}
		}

		/// Destroy all elements in the queue
		void clear()
		{
			// Call destructors of allocated data
			for(uint32_t i = 0; i < m_size; ++i)
			{
				// Redirect because data is not packed thickly, but the heap is.
				m_data[m_heap[i]].data.~DataT();
			}
			m_size = 0;
		}

		/// Change the capacity of the queue and hash map
		void reserve(uint32_t _reserveSize)
		{
			using namespace std;
			if(_reserveSize <= m_capacity)
				return;

			// Save the old raw data and reset this map into empty state by allocating new things.
			DataTupel* oldData = m_data;
			uint32_t* oldHeap = m_heap;
			m_data = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			m_heap = (uint32_t*)malloc(sizeof(uint32_t) * _reserveSize);
			for(uint32_t i = 0; i < _reserveSize; ++i)
				m_data[i].dist = ~0;
			m_capacity = _reserveSize;

			// Move allocated elements with rehashing by re-placing them in the HM.
			// The heap can stay as is, but must be updated to the new positions.
			for(uint32_t i = 0; i < m_size; ++i)
				m_heap[i] = hmAdd(move(oldData[oldHeap[i]].data), i);
				//add(move(oldData[oldHeap[i]].data));

			// Remove old memory (no destruction, because we kept the elements).
			free(oldData);
			free(oldHeap);
		}

		/// Access the minimum element.
		const DataT& min() const
		{
			return m_data[m_heap[0]].data;
		}

		/// Remove the minimum element.
		DataT popMin()
		{
			using namespace std;
			if(m_size > 0)
			{
				// Fast array remove.
				--m_size;
				uint32_t dataIdx = m_heap[0];
				DataT dataTmp = move( m_data[dataIdx].data );
				m_heap[0] = m_heap[m_size];
				// Then repair the heap.
				if(m_size > 0)
				{
					m_data[m_heap[0]].heapIdx = 0;
					bubbleDown(0);
				}

				// Remove the data from the hash map.
				hmRemove(dataIdx);

				return move( dataTmp );
			}
			return DataT();
		}


		/// Add an element and return its unique handle.
		/// If there is another datum with the same content an handle to the old one is
		/// returned. Each key is unique (set).
		template<class _DataT>
		Handle add(_DataT&& _data)
		{
			using namespace std;
			// Resize required?
			if(m_size > 0.77 * m_capacity) {
				reserve(m_size * 2);
			}

			// The heap index will be m_size later...
			uint32_t idx = hmAdd(std::forward<_DataT>(_data), m_size);
			++m_size;

			// Repair the heap.
			bubbleUp(m_size-1);
			return Handle(this, idx);
		}

		/// Delete an arbitrary element from the queue
		void remove(Handle _handle)
		{
			// Remove from heap. We take an element from the back to fill
			// the hole. Since this element was a child before it can only conflict on
			// deeper levels -> bubbleDown.
			--m_size;
			uint32_t freeHeapIdx = m_data[_handle.dataIdx].heapIdx;
			if(freeHeapIdx != m_size)
			{
				m_heap[freeHeapIdx] = m_heap[m_size];
				m_data[m_heap[m_size]].heapIdx = freeHeapIdx;
				bubbleDown(freeHeapIdx);
				assert(m_data[m_heap[m_size]].heapIdx < m_size && "Element moved out of heap!");
				m_heap[m_size] = ~0;
			}
			// Remove from data (hash map)
			m_data[_handle.dataIdx].data.~DataT();
			hmRemove(_handle.dataIdx);
		}

		/// If the priority of a single element was changed, then
		/// call the priorityChanged() method to repair the heap.
		void priorityChanged(Handle _handle)
		{
			assert(_handle.dataIdx < m_capacity && m_data[_handle.dataIdx].heapIdx < m_size && "Out of bounds!");
			// Check both directions, we don't know how the data changed.
			bubbleUp(m_data[_handle.dataIdx].heapIdx);
			bubbleDown(m_data[_handle.dataIdx].heapIdx);
			assert(m_data[_handle.dataIdx].heapIdx < m_size && "Out of bounds!");
		}

		/// Find a contained element or return an invalid handle.
		/// \details If you plan to change the priority of data do not forget to use
		///		priorityChanged or heapify to repair the heap.
		Handle find(const DataT& _data)
		{
			uint32_t d = 0;
			uint32_t h = (uint32_t)m_hash(_data);//hash(reinterpret_cast<const uint32_t*>(&_key), sizeof(_key) / 4);
			uint32_t idx = h % m_capacity;
			while(m_data[idx].dist != ~0 && d <= m_data[idx].dist)
			{
				if(m_keyCompare(m_data[idx].data, _data))
					return Handle(this, idx);
				if(++idx >= m_capacity) idx = 0;
				++d;
			}
			return Handle(nullptr, 0);
		}

		ConstHandle find(const DataT& _data) const
		{
			return const_cast<HashPriorityQueue*>(this)->find(_data);
		}

		/// If the priority of one or multiple elements where changed using [] access
		/// (not changePriority), then heapify() must be called to repair the heap.
		/// The full heapify has a runtime of O(n).
		void heapify()
		{
			for(int32_t i = m_size/2-1; i>=0; --i)
				bubbleDown(i);
		}

		bool empty() const { return m_size == 0; }
		uint32_t size() const { return m_size; }

		ConstHandle operator [] (uint32_t _heapIdx) const { return Handle(this, m_heap[_heapIdx]); }
		Handle operator [] (uint32_t _heapIdx) { return Handle(this, m_heap[_heapIdx]); }

		/// Get an iterator to the first element on the heap.
		Handle begin()
		{
			if(m_size == 0)
				return Handle(nullptr, 0);
			return Handle(this, m_heap[0]);
		}
		ConstHandle begin() const
		{
			if(m_size == 0)
				return ConstHandle(nullptr, 0);
			return ConstHandle(this, m_heap[0]);
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

		struct DataTupel {
			DataT data;
			uint32_t heapIdx;
			uint32_t dist;
		};

		uint32_t* m_heap; // A heap-ordered array of references into the data (hash map index).
		DataTupel* m_data;
		Hash m_hash;
		Compare m_keyCompare;

		void bubbleUp(uint32_t _idx)
		{
			assert(_idx < m_size && "Out of bounds!");
			uint32_t parent = (_idx - 1) / 2;
			while(_idx > 0 && m_data[m_heap[_idx]].data < m_data[m_heap[parent]].data)
			{
				swapElem(parent, _idx);
				_idx = parent;
				parent = (_idx - 1) / 2;
			}
		}

		void bubbleDown(uint32_t _idx)
		{
			assert(_idx < m_size && "Out of bounds!");
			uint32_t child = 2 * _idx + 1;
			// While there are two children...
			while(child + 1 < m_size)
			{
				uint32_t smallest = _idx;
				if(m_data[m_heap[child]].data < m_data[m_heap[_idx]].data) smallest = child;
				if(m_data[m_heap[child+1]].data < m_data[m_heap[smallest]].data) smallest = child + 1;
				if(smallest != _idx)
				{
					swapElem(_idx, smallest);
					_idx = smallest;
					child = 2 * _idx + 1;
				} else return;
			}
			// There could still be another left children
			if(child < m_size && m_data[m_heap[child]].data < m_data[m_heap[_idx]].data)
				swapElem(_idx, child);
		}

		void swapElem(uint32_t _heapIdxA, uint32_t _heapIdxB)
		{
			using namespace std;
			swap(m_heap[_heapIdxA], m_heap[_heapIdxB]);
			m_data[m_heap[_heapIdxA]].heapIdx = _heapIdxA;
			m_data[m_heap[_heapIdxB]].heapIdx = _heapIdxB;
		}

		template<class _DataT>
		uint32_t hmAdd(_DataT&& _data, uint32_t _heapIdx)
		{
			using namespace std;

			DataTupel newDat;
			new (&newDat.data) DataT (std::forward<_DataT>(_data));
			newDat.heapIdx = _heapIdx;
			newDat.dist = 0;
			uint32_t h = (uint32_t)m_hash(newDat.data);//hash(reinterpret_cast<const uint32_t*>(&_key), sizeof(_key) / 4);
			uint32_t idx = h % m_capacity;
			while(m_data[idx].dist != ~0) // while not empty cell
			{
				// return if keys are identically
				if(m_keyCompare(m_data[idx].data, newDat.data))
					return idx;

				if(m_data[idx].dist < newDat.dist) // Swap and then insert the element from this location instead
				{
					swap(newDat, m_data[idx]);
					m_heap[m_data[idx].heapIdx] = idx;	// Keep heap pointers valid
				}
				++newDat.dist;
				//	idx = (idx + 1) % m_capacity;
				if(++idx >= m_capacity) idx = 0;
			}
			// Reached an empty cell -> place
			new (&m_data[idx]) DataTupel (move(newDat));
			m_heap[m_data[idx].heapIdx] = idx;	// Keep heap pointers valid
			return m_heap[_heapIdx]; // Return the address of the new inserted element (not necessarily the last)
		}

		void hmRemove(uint32_t _idx)
		{
			using namespace std;
			uint32_t next = (_idx + 1) % m_capacity;
			while((m_data[next].dist != 0) && (m_data[next].dist != ~0))
			{
				m_data[_idx].data = move(m_data[next].data);
				m_data[_idx].heapIdx = m_data[next].heapIdx;
				m_data[_idx].dist = m_data[next].dist - 1;
				m_heap[m_data[_idx].heapIdx] = _idx;
				_idx = next;
				if(++next >= m_capacity) next = 0;
			}
			m_data[_idx].dist = ~0;
		}
	};

}}