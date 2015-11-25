#pragma once

#include <ei/elementarytypes.hpp>

typedef uint16 half;
half toHalf(float _x);

inline float toFloat(half _x)
{
	uint32 f = ((_x&0x8000)<<16) | (((_x&0x7c00)+0x1C000)<<13) | ((_x&0x03FF)<<13);
	return *((float*)&f);
}