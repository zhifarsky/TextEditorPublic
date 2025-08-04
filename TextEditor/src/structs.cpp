#include <windows.h>
#include "structs.h"
#include "tools.h"

#pragma region Memory Management
void Arena::init(u32 capacity) {
	mem = (u8*)te_malloc(capacity);
	this->capacity = capacity;
	this->size = 0;
	next = NULL;
}

void Arena::release() {
	if (next) {
		next->release();
		te_free(next);
		next = NULL;
	}

	if (mem) {
		te_free(mem);
		mem = NULL;
	}

	capacity = size = 0;
}

void Arena::clear() {
	Arena* arena = this;
	while (arena) {
		arena->size = 0;
		arena = arena->next;
	}
}

void* Arena::alloc(u32 size) {
	if (this->size + size > capacity) {
		// добавляем новый узел
		if (next == NULL) {
			next = (Arena*)te_malloc(sizeof(Arena));
			next->init(max(capacity, size * ARENA_GROW_FACTOR));
		}
		return next->alloc(size);
	}

	u32 oldSize = this->size;
	this->size += size;
	return mem + oldSize;
}

void* Arena::realloc(void* oldMem, u32 oldSize, u32 newSize) {
	u8* newMem = (u8*)alloc(newSize);
	te_memcpy(newMem, oldMem, oldSize);
	return newMem;
}
#pragma endregion