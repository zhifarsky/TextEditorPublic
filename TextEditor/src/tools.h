#pragma once
#include "editor.h"

#if _DEBUG
#define te_assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#else
#define te_assert(Expression)
#endif

#define InRange(value, min_v, max_v) (((value) >= (min_v)) && ((value) <= (max_v)))

void MemCopy(void* destination, const void* source, u64 size) {
	if (destination && source) {
		u8 *dest = (u8*)destination;
		u8 *src = (u8*)source;
		
		while (size--) {
			*dest++ = *src++;
		}
	}
}

#define ZeroStruct(dest) MemZero(&dest, sizeof(dest))
void MemZero(void* destination, u64 size) {
	u8 *dest = (u8*)destination;

	while (size--) {
		*dest++ = 0;
	}
}

void MemSet(void* destination, u8 value, u64 count) {
	u8 *dest = (u8*)destination;

	while (count--) {
		*dest++ = value;
	}
}

u32 StrLen(const char* str) {
	const char* p = str;
	while (*p != '\0') { p++; }
	return p - str;
}

u32 StrLen(const unsigned char* str) {
	return StrLen((const char*)str);
}

s32 StrEqual(const char *strA, const char* strB) {
	u32 lenA = StrLen(strA);
	u32 lenB = StrLen(strB);
	
	if (lenA != lenB)
		return false;
	
	for (u32 i = 0; i < lenA; i++)
	{
		if (strA[i] != strB[i])
			return false;
	}
	
	return true;
} 

#define te_Min(a, b) (a < b ? a : b)
#define te_Max(a, b) (a > b ? a : b)

s32 Abs(s32 value) {
	if (value < 0)
		return -value;
	return value;
}

f32 Pow(f32 x, s32 y) {
	if (y < 0)
		return 1.0f / Pow(x, -y);
	
	f32 result = 1.0;
	for (s32 i = 0; i < y; i++)
		result *= x;

	return result;
}