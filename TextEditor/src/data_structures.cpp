#include "editor.h"
#include "tools.h"

//
// Event queue
//

void Init(event_queue *queue, void* memory, u64 capacity) {
    queue->data = memory;
    queue->size = 0;
    queue->capacity = capacity;
}

void Push(event_queue* queue, void* data, u64 size) {
    if (queue->size + size <= queue->capacity) {
        te_memcpy(&((u8*)queue->data)[queue->size], data, size);
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