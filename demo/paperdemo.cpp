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

	return 0;
}