#pragma once
#include "editor.h"
#include "tools.h"
#include "input.h"

void platform_Print(const char* msg);

//
// Event Queue
//

enum event_type : u8 {
    Event_None = 0,
    Event_Char,
    Event_Key
};

#define EVENT_HEADER event_type eventType;

struct char_event {
    EVENT_HEADER
    u32 utf8CodePoint;
    bool wasDown, isDown;
};

char_event CharEvent(u32 utf8CodePoint, bool wasDown, bool isDown) {
    char_event event;
    event.eventType = Event_Char;
    event.utf8CodePoint = utf8CodePoint;
    event.wasDown = wasDown;
    event.isDown = isDown;
    return event;
}

struct key_event {
    EVENT_HEADER
    te_Key key;
    bool wasDown, isDown;
};

key_event KeyEvent(te_Key key, bool wasDown, bool isDown) {
    key_event event;
    event.key = key;
    event.eventType = Event_Key;
    event.isDown = isDown;
    event.wasDown = wasDown;
    return event;
}

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

//
// String
//

struct string {
    char* base;
    u64 size, capacity;

    char& operator[](u64 i) {
        if (i >= size) {
            platform_Print("string out of bounds\n");
            return base[0];
        }
        return base[i];
    } 
};

#define StrFirst(str) (str.base)
#define StrLast(str) (str.base + str.size - 1)

string String(const char* str) {
    string res;
    res.base = (char*)str;
    res.size = res.capacity = StrLen(str);
    return res;
}

string String(char* base, u64 capacity, u64 size = 0) {
    string str;
    str.base = base;
    str.capacity = capacity;
    str.size = size;
    return str;
}

// NOTE: не проверяет capacity
void IntToString(string* buffer, s32 value) {
    if (buffer->capacity < 1)
        return;
        
    buffer->size = 0;
    
    s32 n = 0, v = Abs(value);
    while(v > 0) {
        v /= 10;
        n++;
    }
    
    n = te_Max(1, n);
    s32 divider = Pow(10, n - 1);

    char* p = buffer->base;
    if (value < 0) {
        *p = '-';
        p++;
        buffer->size++;
    }
    
    v = Abs(value);
        
    for (s32 i = 0; i < n; i++)
    {
        *p = '0' + (v / divider % 10);
        divider /= 10;
        p++;
        buffer->size++;
    }
}

void Concat(string* a, string* b) {
    if (a->size + b->size > a->capacity)
        return;
        
    CopyMem(a->base + a->size, b->base, b->size);
    a->size = a->size + b->size;
}

void ToCString(char* buffer, u64 bufferSize, string* str) {
    if (bufferSize < str->size + 1)
        return;
        
    CopyMem(buffer, str->base, str->size);
    buffer[str->size] = 0;
}

// void IntToString(char *buffer, s32 value) {
//     s32 n = 0, v = Abs(value);
//     while(v > 0) {
//         v /= 10;
//         n++;
//     }
    
//     n = te_Max(1, n);
//     s32 divider = Pow(10, n - 1);

//     char* p = buffer;
//     if (value < 0) {
//         *p = '-';
//         p++;
//     }
    
//     v = Abs(value);
        
//     for (s32 i = 0; i < n; i++)
//     {
//         *p = '0' + (v / divider % 10);
//         divider /= 10;
//         p++;
//     }
    
//     *p = 0;
// }