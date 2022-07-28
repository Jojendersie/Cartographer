#pragma once

#include <cstdint>

namespace ca::pa::utf8 {

/// Check if the first 1 to 4 bytes form a valid UTF-8 code point
inline bool isValidCodePoint(const char* _str)
{
	if((*_str & 0x80) == 0) return true;
	if((*reinterpret_cast<const uint16_t*>(_str) & 0xe0c0) == 0xc080) return true;
	const uint32_t u32 = *reinterpret_cast<const uint32_t*>(_str);
	return ((u32 & 0xf8c0c000) == 0xf0808000) || ((u32 & 0xf8c0c0c0) == 0xf0808080);
}


/// Get the number of bytes of the first code point. Returns -1 for invalid codes.
inline int codeSize(const char* _str)
{
	const uint32_t u32 = *reinterpret_cast<const uint32_t*>(_str);
	if((*_str & 0x80) == 0) return 1;
	if((*reinterpret_cast<const uint16_t*>(_str) & 0xe0c0) == 0xc080) return 2;
	if((u32 & 0xf8c0c000) == 0xf0808000) return 3;
	if((u32 & 0xf8c0c0c0) == 0xf0808080) return 4;
	return -1;
}

// For entire string implement https://lemire.me/blog/2020/10/20/ridiculously-fast-unicode-utf-8-validation/


inline  bool isIntermediateByte(char _c)
{
	return (_c & 0xc0) == 0x80;
}

// Got to the next code point
inline size_t next(const char* _text, size_t _pos)
{
	if(_text[_pos] == 0) return _pos;
	++_pos;
	while(isIntermediateByte(_text[_pos])) ++_pos;
	return _pos;
}

// Go to the previous code point
inline size_t prev(const char* _text, size_t _pos)
{
	if(_pos == 0) return 0;
	--_pos;
	while(_pos > 0 && isIntermediateByte(_text[_pos])) --_pos;
	return _pos;
}


/// Check if the first character in the string is a digit in 0-9
inline bool isDigit(const char* _str)
{
	return *_str >= '0' && *_str <= '9';
}

/// Check if the first character in the string is - or +
inline bool isSign(const char* _str)
{
	return *_str == '-' || *_str == '+';
}

} // namespace ca::pa::utf8