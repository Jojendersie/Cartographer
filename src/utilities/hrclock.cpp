#include "ca/cc/utilities/hrclock.hpp"

HRClock::HRClock() : m_timeStamp(now())
{
}

double HRClock::deltaTime()
{
	double oldTime = m_timeStamp;
	m_timeStamp = now();
	return m_timeStamp - oldTime;
}

// ********** Platform dependent code *************
#if defined(_WIN32) || defined(WIN32)

#include <windows.h>

const double QUERY_FREQUENCY = []() -> double
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart / 1000.0;
}();
const long long START_TIMEPOINT = []() -> long long
{
	LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
	return count.QuadPart;
}();

double HRClock::now()
{
	LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
	return (count.QuadPart - START_TIMEPOINT) / QUERY_FREQUENCY;
}

#else

#include <chrono>

const std::chrono::high_resolution_clock::time_point START_TIMEPOINT = []()
{
	return std::chrono::high_resolution_clock::now();
}();

double HRClock::now()
{
	auto time = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds delta = time - START_TIMEPOINT;
	return delta.count() / 1000000.0;
}

#endif