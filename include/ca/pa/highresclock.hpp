#pragma once

/// Plattform independent high resolution clock.
///
/// This uses QueryPerformance counter with windows and
///	std::chrono::high_resolution_clock on other systems.
class HRClock
{
public:
	HRClock();

	/// Get time since program start in milliseconds
	/// The behavior during creation of global variables is undefined.
	static double now();

	/// Get time since last call or object initialization in milliseconds.
	double deltaTime();

private:
	double m_timeStamp;	/// Time since last call
};