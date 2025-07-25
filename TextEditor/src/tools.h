#pragma once
#include <windows.h>

void ErrorHandle();
void WarningHandle(const char* msg = "ERROR");
void DebugWarningHandle(const char* msg = "ERROR");
void SaveToFileWin32(const char* filename, const char* buffer, int bufferSize);

int te_strlen(const char* str);

void te_puts(const char* str);

void* te_malloc(size_t size);
void* te_calloc(size_t count, size_t size);
void* te_realloc(void* oldMem, size_t oldSize, size_t newSize);
void te_free(void* mem);
void te_memcpy(void *destination, void *source, size_t size);

int te_min(int a, int b);
int te_max(int a, int b);
float te_min(float a, float b);
float te_max(float a, float b);

float te_pow(float a, int b);

bool PickFile(HWND window, char* outFilename, int outFilenameMaxSize);
bool PickNewFile(HWND window, char *outFilename, int outFilenameMaxSize);

bool IsLetter(char c);

#define te_IsKeyDown(vkey) (GetAsyncKeyState(vkey) & 0x8000) != 0

#define te_assert(expr) \
  do { \
    if (!(expr)) { \
      OutputDebugStringA("Assertion failed: " #expr "\n"); \
      DebugBreak(); \
      __debugbreak(); \
    } \
  } while (0);