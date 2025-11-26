#pragma once
#include "editor.h"

#if _DEBUG
#define te_assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#else
#define te_assert(Expression)
#endif


void CopyMem(void* destination, void* source, u64 size);
#define ZeroStruct(dest) ZeroMem(&dest, sizeof(dest))
void SetMem(void* destination, u8 value, u64 count);

void CopyMem(void* destination, void* source, u64 size) {
    u8 *dest = (u8*)destination;
    u8 *src = (u8*)source;

    while (size--) {
        *dest++ = *src++;
    }
}

void ZeroMem(void* destination, u64 size);
void ZeroMem(void* destination, u64 size) {
    u8 *dest = (u8*)destination;

    while (size--) {
        *dest++ = 0;
    }
}

void SetMem(void* destination, u8 value, u64 count) {
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