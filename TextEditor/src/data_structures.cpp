#include "editor.h"
#include "tools.h"

//
// Event queue
//

void Init(event_queue *queue, void* base, u64 capacity) {
    queue->base = base;
    queue->size = 0;
    queue->capacity = capacity;
}

void Push(event_queue* queue, void* data, u64 size) {
    if (queue->size + size <= queue->capacity) {
        te_memcpy(&((u8*)queue->base)[queue->size], data, size);
        queue->size += size;
    }
}

void Clear(event_queue *queue) {
    queue->size = 0;
}

char_event CharEvent(u32 utf8CodePoint) {
    char_event event;
    event.eventType = Event_Char;
    event.utf8CodePoint = utf8CodePoint;
    return event;
}

//
// Arena
//

void Init(memory_arena* arena, void* base, u64 capacity) {
    arena->base = (u8*)base;
    arena->size = 0;
    arena->capacity = capacity;
}

void* Push(memory_arena* arena, u64 size) {
    te_assert(arena->size + size <= arena->capacity);

    u8* result = arena->base + arena->size;
    arena->size += size;

    return result;
}

void Clear(memory_arena* arena) {
    arena->size = 0;
}