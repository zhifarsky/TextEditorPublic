#pragma once
#include "editor.h"

#if _DEBUG
#define te_assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#else
#define te_assert(Expression)
#endif

void te_memcpy(void* destination, void* source, u64 size);