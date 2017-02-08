#pragma once

namespace ca { namespace pa {

	/// Min-heap implementation of a priority queue.
	template<typename PriorityT, typename DataT>
	class PriorityQueue
	{
	public:
		explicit PriorityQueue(uint32_t _reserveSize) :
			m_capacity(_reserveSize),
			m_size(0)
		{
			m_data = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			m_heap = (uint32_t*)malloc(sizeof(uint32_t) * _reserveSize);
		}

		PriorityQueue(PriorityQueue&& _other) :
			m_capacity(_other.m_capacity),
			m_size(_other.m_size),
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
					m_data[i].p.~PriorityT();
					m_data[i].d.~DataT();
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

			DataTupel* newMem = = (DataTupel*)malloc(sizeof(DataTupel) * _reserveSize);
			for(uint32_t i = 0; i < m_size; ++i)
				new (&newMem[i])(PriorityT)(move(m_data[i]));

			m_heap = (uint32_t*)realloc(m_heap, sizeof(uint32_t), _reserveSize);

			free(m_data);
			free(m_heap);
			m_data = newMem;
			m_capacity = _reserveSize;
		}

		/// Access the minimum element.
		const DataT& min() const
		{
			return m_data[m_heap[0]].d;
		}

		/// Remove the minimum element.
		DataT popMin()
		{
			using namespace std;
			if(m_size > 0)
			{
				// Fast array remove.
				--m_size;
				DataT tmp( move( m_data[0] ) );
				m_data[0] = m_data[m_size];
				// Then repair the heap.
				bubbleDown(0);
				return tmp;
			}
			return DataT();
		}

		/// Unique handle of an inserted element
		typedef uint32_t Handle;

		/// Add an element and return its unique handle.
		/// If there is another datum with the same content both will have their own
		/// instances and locations. There is no set mechanism.
		template<class _PriorityT, class _DataT>
		Handle add(_PriorityT&& _priority, _DataT&& _data)
		{
			// Place element into array.
			new (&m_data[m_size].p)(PriorityT)(std::forward<_PriorityT>(_priority));
			new (&m_data[m_size].d)(DataT)(std::forward<_DataT>(_data));
			m_data[m_size].heapIdx = m_size;
			// Repair the heap.
			m_heap[m_size] = m_size;
			bubbleUp(m_size);
			return m_size++;
		}

		PriorityT getPriority(Handle _handle) const
		{
			return m_data[_handle].p;
		}

		void changeKey(Handle _handle, PriorityT _newPriority)
		{
			PriorityT oldPriority = m_data[_handle].p;
			m_data[_handle].p = _newPriority;
			if(_newPriority < oldPriority)
				bubbleUp(m_data[_handle].heapIdx);
			else
				bubbleDown(m_data[_handle].heapIdx);
		}

	private:
		uint32_t m_capacity;
		uint32_t m_size;

		struct DataTupel {
			DataT d;
			PriorityT p;
			uint32_t heapIdx;
		};

		uint32_t* m_heap; // A heap-ordered array of references into the data.
		DataTupel* m_data;

		void bubbleUp(uint32_t _idx)
		{
			uint32_t parent = (_idx - 1) / 2;
			while(_idx > 0 && m_data[m_heap[parent]].p > m_data[m_heap[_idx]].p)
			{
				swapElem(parent, _idx);
				_idx = parent;
				parent = (_idx - 1) / 2;
			}
		}

		void bubbleDown(uint32_t _idx)
		{
			uint32_t child = 2 * _idx;
			// While there are two children...
			while(child + 1 < m_size)
			{
				uint32_t smallest = _idx;
				if(m_data[m_heap[child]].p < m_data[m_heap[_idx]].p) smallest = child;
				if(m_data[m_heap[child+1]].p < m_data[m_heap[smallest]].p) smallest = child + 1;
				if(smallest != _idx)
				{
					swapElem(_idx, smallest);
					_idx = smallest;
				}
			}
			// There could still be another left children
			if(child < m_size && m_data[m_heap[child]].p < m_data[m_heap[_idx]].p)
				swapElem(_idx, child);
		}

		void swapElem(uint32_t _heapIdxA, uint32_t _heapIdxB)
		{
			using namespace std;
			swap(m_heap[_heapIdxA], m_heap[_heapIdxB]);
			m_data[m_heap[_heapIdxA]].heapIdx = _heapIdxA;
			m_data[m_heap[_heapIdxB]].heapIdx = _heapIdxB;
		}
	};

}}