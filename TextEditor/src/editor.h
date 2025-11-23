#pragma once
#include <stdint.h>

#include <GL/gl.h>
#include <GL/wglext.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_win32.h> // TODO: убрать. зависит от платформы

#define Kilabytes(n) (n * 1024LL)
#define Megabytes(n) (Kilabytes(n) * 1024LL)
#define Gigabytes(n) (Megabytes(n) * 1024LL)

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

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
    void *data;
    u64 size, capacity;
};
void Init(event_queue *queue, void* memory, u64 capacity);
void Push(event_queue *queue, void* data, u64 size);
void Clear(event_queue *queue);
#define PUSH_EVENT(queue, event) Push(&queue, &event, sizeof(event))

//
// Platform Services
//

void platform_Print(const char* message);

//
// Editor services
//

struct program_memory {
    void * stub;
};

struct program_input {
    void * stub;
};

void EditorInit();
void EditorUpdateAndRender(program_memory* memory, event_queue* eventQueue, program_input* input);
