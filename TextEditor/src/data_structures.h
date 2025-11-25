#pragma once
#include "editor.h"

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
char_event CharEvent(u32 utf8CodePoint);

struct key_event {
    EVENT_HEADER
    u32 key; // TODO: enum, чтобы не зависеть от платформы?
    bool wasDown, isDown;
};

struct event_queue {
    void *base;
    u64 size, capacity;
};
void Init(event_queue *queue, void* memory, u64 capacity);
void Push(event_queue *queue, void* data, u64 size);
void Clear(event_queue *queue);
#define PUSH_EVENT(queue, event) Push(&queue, &event, sizeof(event))

//
// Arena
//

// TODO: посмотреть про temp arena, scratch arena

struct memory_arena {
    u8* base;
    u64 size, capacity;
};

void Init(memory_arena* arena, void* base, u64 capacity);
void* Push(memory_arena* arena, u64 size);
void Clear(memory_arena* arena);