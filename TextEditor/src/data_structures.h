#pragma once
#include "editor.h"
#include "tools.h"

//
// Event Queue
//

enum event_type : u8 {
    Event_None = 0,
    Event_Char,
    Event_KeyDown
};

#define EVENT_HEADER event_type eventType;

struct char_event {
    EVENT_HEADER
    u32 utf8CodePoint;
};

char_event CharEvent(u32 utf8CodePoint) {
    char_event event;
    event.eventType = Event_Char;
    event.utf8CodePoint = utf8CodePoint;
    return event;
}

struct key_event {
    EVENT_HEADER
    u32 key; // TODO: enum, чтобы не зависеть от платформы?
    bool wasDown, isDown;
};

struct event_queue {
    void *base;
    u64 size, capacity;
};

void Init(event_queue *queue, void* base, u64 capacity) {
    queue->base = base;
    queue->size = 0;
    queue->capacity = capacity;
}

#define PUSH_EVENT(queue, event) Push(&queue, &event, sizeof(event))
void Push(event_queue* queue, void* data, u64 size) {
    if (queue->size + size <= queue->capacity) {
        CopyMem(&((u8*)queue->base)[queue->size], data, size);
        queue->size += size;
    }
}

void Clear(event_queue *queue) {
    queue->size = 0;
}

//
// Arena
//

// TODO: посмотреть про temp arena, scratch arena

struct memory_arena {
    u8* base;
    u64 size, capacity;
};

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