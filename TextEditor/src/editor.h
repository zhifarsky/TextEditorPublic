#pragma once
#include <stdint.h>

#define Kilobytes(n) (n * 1024LL)
#define Megabytes(n) (Kilobytes(n) * 1024LL)
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

#include "data_structures.h"

//
// Platform Services
//

void platform_Print(const char* message);

void platform_StartFrame();
void platform_EndFrame();

void* platform_debug_Malloc(u64 size);
void* platform_debug_Realloc(void* oldMem, u64 oldSize, u64 newSize);
void platform_debug_Free(void* memory);

//
// Editor services
//

struct permanent_storage {
	void *base;
	u64 capacity;
	
	memory_arena arena; 
};

struct transient_storage {
	void* base;
	u64 capacity;
	
	memory_arena frameArena; // очищается в начале каждого кадра
};

struct program_memory {
		permanent_storage permStorage;
		transient_storage tranStorage;
		
		bool isInitialized;
};

struct program_input {
		void * stub;
};

void EditorUpdateAndRender(program_memory* memory, event_queue* eventQueue, program_input* input);
