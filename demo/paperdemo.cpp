#include "ca/paper.hpp"
#include <iostream>

using namespace ca;
using namespace pa;

class OutputGuard: public ReferenceCountable
{
	static int s_nextObjectID;
protected:
	int m_id;
public:
	OutputGuard() :
		m_id(s_nextObjectID++)
	{
		std::cout << "Created guard object #" << m_id << '\n';
	}

	~OutputGuard()
	{
		std::cout << "Deleted guard object #" << m_id << '\n';
	}

	virtual void tell() const
	{
		std::cout << '#' << m_id << " tell called!\n";
	}
};
int OutputGuard::s_nextObjectID = 0;

class OutputGuardB: public OutputGuard
{
public:
	virtual void tell() const override
	{
		std::cout << '#' << m_id << " super guard tell called!\n";
	}
};


struct QuickHash
{
	uint32_t operator () (int _x) const
	{
		return _x;
	}
};
uint32_t hashint( uint32_t a)
{
	a -= (a<<6);
	a ^= (a>>17);
	a -= (a<<9);
	a ^= (a<<4);
	a -= (a<<3);
	a ^= (a<<10);
	a ^= (a>>15);
	return a;
}

//#define STD_MAP_BENCH
#ifdef STD_MAP_BENCH
#include <unordered_map>
#endif
void hashMapBenchmark()
{
	HRClock clock;
	double totalTime;

	// Sequential insertion
	if(true)
	{
#ifdef STD_MAP_BENCH
		std::unordered_map<int, int, QuickHash> map;
#else
		HashMap<int, int, QuickHash> map;
#endif

		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
#ifdef STD_MAP_BENCH
			map.emplace(s, s);
#else
			//map.add(s, s);
			map[s] = s;
#endif
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Insertion of 8M elements (integer sequence): ", totalTime, " ms.");

		// Test if all elements are contained
		clock.deltaTime();
		for(int i = 0; i < 1024 * 8192; ++i)
#ifdef STD_MAP_BENCH
			if(map.find(i) == map.end()) 
#else
			if(!map.find(i))
#endif
				logError("Cannot find an element which should be there!");
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Finding of 8M contained elements (integer sequence): ", totalTime, " ms.");

		// Test for a lot of non-contained elements
		clock.deltaTime();
		for(int i = 1024 * 8192; i < 2 * 1024 * 8192; ++i)
#ifdef STD_MAP_BENCH
			if(map.find(i) != map.end()) 
#else
			if(map.find(i))
#endif
				logError("Found an element which should not be there!");
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Finding of 8M non-contained elements: ", totalTime, " ms.");

		// Test of the clear call
		clock.deltaTime();
		map.clear();
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Time to clear the map: ", totalTime, " ms.");
	}

	// Random insertion + deletion
	if(true)
	{
#ifdef STD_MAP_BENCH
		std::unordered_map<int, int, QuickHash> map;
#else
		HashMap<int, int, QuickHash> map;
#endif

		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
			int x = hashint(s);
#ifdef STD_MAP_BENCH
			map.emplace(x, x);
#else
			map.add(x, x);
#endif
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Insertion of 8M randomized integer elements: ", totalTime, " ms.");

		// Random deletion
		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
			int x = hashint(s);
#ifdef STD_MAP_BENCH
			map.erase(x);
#else
			map.remove(x);
#endif
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Deletion of 8M randomized integer elements: ", totalTime, " ms.");
	}
}


void priorityQueueBenchmark()
{
	HRClock clock;
	double totalTime;
	PriorityQueue<int> queue;

	// Add elements
	clock.deltaTime();
	for(int s = 0; s < 1024 * 2048; ++s)
	{
		int x = hashint(s);
		queue.add(x % 1357);
	}
	totalTime = clock.deltaTime();
	logInfo("[PQ Benchmark] Insertion of 2M elements with random priorities: ", totalTime, " ms.");

	//if(!queue.isHeap())
		//logError("Corrupt heap after insertion!");

	// Change some priorities
	clock.deltaTime();
	for(int s = 0; s < 1024 * 1024; ++s)
	{
		// The handles are indices -> we can create random handles.
		uint32_t x = hashint(s);
		uint32_t h = x & 0x1fffff;
		queue.changePriority(queue[h], int(x >> 21));
	}
	totalTime = clock.deltaTime();
	logInfo("[PQ Benchmark] Changing priority of 1M elements: ", totalTime, " ms.");

	//if(!queue.isHeap())
		//logError("Corrupt heap after changing priorities!");

	// Pop all elements and check their order
	clock.deltaTime();
	int prevPriority = queue.min();
	while(!queue.empty())
	{
		int priority = queue.min();
		if(priority < prevPriority)
			logError("Invalid priority order. Queue is corrupt!");
//		if(prevPriority != priority)
	//		logInfo(prevPriority);
		prevPriority = priority;
		queue.popMin();
	}
	totalTime = clock.deltaTime();
	logInfo("[PQ Benchmark] Poping all 2M elements in order: ", totalTime, " ms.");
}



int main()
{
	// Output some log messages
	logPedantic("This message should only appear on PEDANTIC level.");
	logInfo("The log can convert a list of parameters easily: ", 42, " ", 3.1415f, ".");
	logWarning("The log allocates a string in each call.");
	logError(std::string("Something went wrong!"));
	try {
		logFatal("Badly wrong!");
	} catch(...) {
		std::cerr << "Fatal error caused exception.\n";
	}

	// The valid output for the program is:
	// Created #0
	// Created #1
	// Created #2
	// Created #3
	// Deleted #1
	// Deleted #3
	// #0 tell
	// Deleted #0
	// ptr0 lost
	// #2 tell
	// Deleted #2
	{
		RefPtr<OutputGuard> ptr0(new OutputGuard());
		RefPtr<OutputGuard> ptr1(new OutputGuard());
		{
			RefPtr<OutputGuard> ptr2(new OutputGuard());
			RefPtr<OutputGuard> ptr3(new OutputGuard());
			ptr1 = ptr2;
		}
		RefPtr<OutputGuard> ptr4;
		ptr4 = ptr1;
		{
			RefPtr<OutputGuard> ptr5 = std::move(ptr0);
			(*ptr5).tell();
		}
		if(!ptr0)
			std::cout << "ptr0 lost its reference by move.\n";
		ptr1->tell();
	}

	{
		RefPtr<OutputGuardB> ptr0(new OutputGuardB());
		// Static cast
		RefPtr<OutputGuard> ptr1 = ptr0;
		ptr1->tell();
		// Dynamic cast
		RefPtr<OutputGuardB> ptr2 = ptr1;
		ptr2->tell();
	}

	hashMapBenchmark();
	priorityQueueBenchmark();

	return 0;
}