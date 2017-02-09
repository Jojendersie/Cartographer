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

void hashMapBenchmark()
{
	HRClock clock;
	double totalTime;

	// Sequential insertion
	if(true)
	{
		HashMap<int, int, QuickHash> map;

		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
			map.add(s, s);
			//map.emplace(s, s);
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Insertion of 8M elements (integer sequence): ", totalTime, " ms.");

		// Test if all elements are contained
		clock.deltaTime();
		for(int i = 0; i < 1024 * 8192; ++i)
			if(!map.find(i)) 
				logError("Cannot find an element which should be there!");
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Finding of 8M contained elements (integer sequence): ", totalTime, " ms.");

		// Test for a lot of non-contained elements
		clock.deltaTime();
		for(int i = 1024 * 8192; i < 2 * 1024 * 8192; ++i)
			if(map.find(i))
				logError("Found an element which should not be there!");
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Finding of 8M non-contained elements: ", totalTime, " ms.");
	}

	// Random insertion + deletion
	if(true)
	{
		HashMap<int, int, QuickHash> map;

		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
			int x = hashint(s);
			map.add(x, x);
			//map.emplace(x, x);
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Insertion of 8M randomized integer elements: ", totalTime, " ms.");

		// Random deletion
		clock.deltaTime();
		for(int s = 0; s < 1024 * 8192; ++s)
		{
			int x = hashint(s);
			map.remove(x);
			//map.emplace(x, x);
		}
		totalTime = clock.deltaTime();
		logInfo("[HM Benchmark] Deletion of 8M randomized integer elements: ", totalTime, " ms.");
	}
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

	return 0;
}