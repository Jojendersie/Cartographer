#pragma once

namespace ca { namespace pa {

	/// Min-heap implementation of a priority queue.
	/// \tparam DataT Type of the data to be stored. Must support the operator less <.
	///		Further, there must be an 'PriorityT updatePriority(DataT&, PriorityT)' method
	///		which changes the key of a datum and returns its previous priority. DataT and
	///		PriorityT can be of the same type if there is no difference between elements
	///		and priorities. There is a standard implementation which uses move() for
	///		this case.
	template<typename DataT>
	class PriorityQueue
	{
	public:
		explicit PriorityQueue(uint32_t _reserveSize = 32) :
			m_capacity(_reserveSize),
			m_size(0),
			m_nextFreeData(0)
		{
			m_data = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			m_heap = (uint32_t*)malloc(sizeof(uint32_t) * _reserveSize);
			// Create the next free element list
			for(uint32_t i = 0; i < m_capacity; ++i)
				m_data[i].heapIdx = i + 1;
		}

		PriorityQueue(PriorityQueue&& _other) :
			m_capacity(_other.m_capacity),
			m_size(_other.m_size),
			m_nextFreeData(_other.m_nextFreeData),
			m_data(_other.m_data),
			m_heap(_other.m_heap)
		{
			_other.m_data = nullptr;
			_other.m_heap = nullptr;
		}

		PriorityQueue& operator = (PriorityQueue&& _rhs)
		{
			this->~PriorityQueue();
			m_capacity = _rhs.m_capacity;
			m_size = _rhs.m_size;
			m_nextFreeData = _rhs.m_nextFreeData;
			m_data = _rhs.m_data;
			m_heap = _rhs.m_heap;
			_rhs.m_data = nullptr;
			_rhs.m_heap = nullptr;
			return *this;
		}


		~PriorityQueue()
		{
			if(m_data)
			{
				// Call destructors of allocated keys and data
				for(uint32_t i = 0; i < m_size; ++i)
				{
					// Redirect because data is not packed thickly, but the heap is.
					m_data[m_heap[i]].data.~DataT();
				}
				free(m_data);
				free(m_heap);
			}
		}

		/// Change the capacity of the queue
		void reserve(uint32_t _reserveSize)
		{
			using namespace std;
			if(_reserveSize <= m_capacity)
				return;

			DataTupel* newMem = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			// Copy only allocated elements
			for(uint32_t i = 0; i < m_size; ++i)
			{
				new (&newMem[m_heap[i]].data)(DataT)(move(m_data[m_heap[i]].data));
				newMem[i].heapIdx = m_data[i].heapIdx;
			}
			// Copy free list/heap indices for all elements
			for(uint32_t i = 0; i < m_capacity; ++i)
				newMem[i].heapIdx = m_data[i].heapIdx;
			// Initialize new free list entries
			for(uint32_t i = m_capacity; i < _reserveSize; ++i)
				newMem[i].heapIdx = i + 1;
			free(m_data);
			m_data = newMem;

			m_heap = (uint32_t*)realloc(m_heap, sizeof(uint32_t) * _reserveSize);

			m_capacity = _reserveSize;
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
				m_heap[0] = m_heap[m_size];
				// Then repair the heap.
				bubbleDown(0);
				// The data is not moved, but the current cell is free now.
				m_data[dataIdx].heapIdx = m_nextFreeData;
				m_nextFreeData = dataIdx;
				return move( m_data[dataIdx].data );
			}
			return DataT();
		}

		/// Unique handle of an inserted element
		typedef uint32_t Handle;

		/// Add an element and return its unique handle.
		/// If there is another datum with the same content both will have their own
		/// instances and locations. There is no set mechanism.
		template<class _DataT>
		Handle add(_DataT&& _data)
		{
			if(m_nextFreeData == m_capacity)
				reserve(2 * m_capacity);
			// Place element into array.
			new (&m_data[m_nextFreeData].data)(DataT)(std::forward<_DataT>(_data));
			uint32_t newNextFreeData = m_data[m_nextFreeData].heapIdx;
			m_data[m_nextFreeData].heapIdx = m_size;
			// Repair the heap.
			m_heap[m_size] = m_nextFreeData;
			m_nextFreeData = newNextFreeData;
			bubbleUp(m_size);
			return m_size++;
		}

		DataT get(Handle _handle) const
		{
			return m_data[_handle].data;
		}

		template<class _PriorityT>
		void changePriority(Handle _handle, _PriorityT&& _newPriority)
		{
			using namespace std;
			_PriorityT oldPriority = updatePriority(m_data[_handle].data, std::forward<_PriorityT>(_newPriority));
			if(_newPriority < oldPriority)
				bubbleUp(m_data[_handle].heapIdx);
			else
				bubbleDown(m_data[_handle].heapIdx);
		}

		bool empty() const { return m_size == 0; }

		// TEST DEBUG STUFF
	/*	bool isHeap() const
		{
			for(uint32_t i = 1; i < m_size; ++i)
			{
				uint32_t parent = (i - 1) / 2;
				if(m_data[m_heap[i]].p < m_data[m_heap[parent]].p)
					return false;
			}
			return true;
		}*/

	private:
		uint32_t m_capacity;
		uint32_t m_size;

		struct DataTupel {
			DataT data;
			uint32_t heapIdx;
		};

		uint32_t* m_heap; // A heap-ordered array of references into the data.
		DataTupel* m_data;
		uint32_t m_nextFreeData;	// Free-list memory for the data (to guarantee constant positions on delete)

		void bubbleUp(uint32_t _idx)
		{
			uint32_t parent = (_idx - 1) / 2;
			while(_idx > 0 && m_data[m_heap[parent]].data > m_data[m_heap[_idx]].data)
			{
				swapElem(parent, _idx);
				_idx = parent;
				parent = (_idx - 1) / 2;
			}
		}

		void bubbleDown(uint32_t _idx)
		{
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

		template<typename T>
		T updatePriority(T& _data, T&& _newPriority)
		{
			using namespace std;
			T tmp( move(_data) );
			new (&_data)(T)(std::forward<T>(_newPriority));
			return move(tmp);
		}
	};

}}