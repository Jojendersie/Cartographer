#include "memory/refptr.hpp"
#include <iostream>

using namespace ca;
using namespace pa;

class OutputGuard: public ReferenceCountable
{
	static int s_nextObjectID;
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

	void tell() const
	{
		std::cout << '#' << m_id << " tell called!\n";
	}
};
int OutputGuard::s_nextObjectID = 0;

int main()
{
	// The valid output for the program is:
	// Created #0
	// Created #1
	// Created #2
	// Created #3
	// Deleted #1
	// Deleted #3
	// #0 tell
	// Deleted #0
	// #2 tell
	// Deleted #2
	{
		RefPtr<OutputGuard> ptr0(new OutputGuard());
		RefPtr<OutputGuard> ptr1 = new OutputGuard();
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
		ptr1->tell();
	}

	return 0;
}