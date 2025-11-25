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
#pragma optimize("", off)
void ZeroMem(void* destination, u64 size) {
    u8 *dest = (u8*)destination;

    while (size--) {
        *dest++ = 0;
    }
}
#pragma optimize("", on)

#pragma optimize("", off)
void SetMem(void* destination, u8 value, u64 count) {
    u8 *dest = (u8*)destination;

    while (count--) {
        *dest++ = value;
    }
}
#pragma optimize("", on)