#include "utilities/half.hpp"

// Code from glm:
half toHalf(float _x)
{
	int i = *reinterpret_cast<const int*>(&_x);

	// Disassamble into sign, exponent and mantissa
	int s =  (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m =   i        & 0x007fffff;

	if(e <= 0) // Denormalized half or 0
	{
		// _value smaller then smallest representable half -> convert to signed zero
		if(e < -10)
			return uint16(s);

		// _value small -> denormalized half
		m = (m | 0x00800000) >> (1 - e);

		// Round to nearest (even)
		if((m & 0x00003000) == 0x00003000) 
			m += 0x00002000;

		// Assemble the half from s, e (zero) and m.
		return uint16(s | (m >> 13));
	}
	else if(e == 0xff - (127 - 15)) // NaN or infinity
	{
		// _value is an infinity
		if(m == 0)
			return uint16(s | 0x7c00);
		else
		{
			// _value is NaN, preserve most bits
			m >>= 13;
			return uint16(s | 0x7c00 | m | (m == 0));
		}
	}
	else // Standard case
	{
		// Round to even
		if((m & 0x00003000) == 0x00003000)
		{
			m += 0x00002000;
			if(m & 0x00800000)
			{
				m =  0;     // overflow in significand,
				e += 1;     // adjust exponent
			}
		}

		// Handle exponent overflow
		if (e > 30)
		{
			volatile float o = 1e30f;        // Cause a hardware floating point overflow;
			o *= o;

			return uint16(s | 0x7c00);
			// if this returns, the half becomes an
		}   // infinity with the same sign as f.

			// Assemble the half from s, e and m.
		return uint16(s | (e << 10) | (m >> 13));
	}
}