#pragma once

typedef uint16 half;
inline half toHalf(float _x)
{
	uint32 x = *((uint32*)&_x);
	return ((x>>16)&0x8000)|((((x&0x7f800000)-0x38000000)>>13)&0x7c00)|((x>>13)&0x03ff);
}

inline float toFloat(half _x)
{
	uint32 f = ((_x&0x8000)<<16) | (((_x&0x7c00)+0x1C000)<<13) | ((_x&0x03FF)<<13);
	return *((float*)&f);
}