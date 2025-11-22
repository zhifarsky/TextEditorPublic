#pragma once
#include <stdint.h>

#include <GL/gl.h>
#include <GL/wglext.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_win32.h>

typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

struct program_memory {
    void * stub;
};

struct program_input {
    void * stub;
};