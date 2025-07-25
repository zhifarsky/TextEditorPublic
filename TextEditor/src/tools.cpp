#include <windows.h>
#include "tools.h"

void ErrorHandle() {
	DWORD errorCode = GetLastError();
	OutputDebugStringA("\nERROR\n");
	ExitProcess(1);
}

void WarningHandle(const char* msg) {
	DWORD errorCode = GetLastError();
	MessageBoxA(NULL, msg, "Error", MB_OK);
}

void DebugWarningHandle(const char *msg)
{
	OutputDebugStringA(msg);
}

void SaveToFileWin32(const char* filename, const char* buffer, int bufferSize) {
  HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    WarningHandle("Error on opening file");
    return;
  }

  if (!WriteFile(hFile, buffer, bufferSize, NULL, NULL)) {
    WarningHandle("Error on writing file");
  }
  CloseHandle(hFile);
}

int te_strlen(const char* str) {
	int count = 0;
	while (str[count] != '\0')
		count++;
	return count;
}

void te_puts(const char* str) {
	HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		WriteConsoleA(stdOut, str, te_strlen(str), &written, NULL);
	}
}

void* te_malloc(size_t size) {
	return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void* te_calloc(size_t count, size_t size) {
	char* mem = (char*)VirtualAlloc(0, count * size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	// TDOO: может VirtualAlloc сразу возвращать зануленную память?
	for (size_t i = 0; i < count * size; i++) {
		mem[i] = 0;
	}
	return mem;
}

void* te_realloc(void* oldMem, size_t oldSize, size_t newSize) {
	void* mem = VirtualAlloc(0, newSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	te_memcpy(mem, oldMem, oldSize);
	VirtualFree(oldMem, 0, MEM_RELEASE);

	return mem;
}

void te_free(void* mem) {
	VirtualFree(mem, 0, MEM_RELEASE);
}

void te_memcpy(void *destination, void *source, size_t size) {
	for (size_t i = 0; i < size; i++) {
		((char*)destination)[i] = ((char*)source)[i];
	}
}

int te_min(int a, int b) {
  if (a < b) return a;
  return b;
}

int te_max(int a, int b) {
  if (a > b) return a;
  return b;
}

float te_min(float a, float b) {
  if (a < b) return a;
  return b;
}

float te_max(float a, float b) {
  if (a > b) return a;
  return b;
}

// TODO: есть вариант лучше?
float te_pow(float a, int b)
{
	if (b == 0)
		return 1.0f;

  float res = a;

	// положительная степень
	if (b > 0) {
		for (int i = 1; i < b; i++)
			res *= a;
	}
	// отрицательая степень
	else {
		for (int i = 1; i > b; i--)
			res /= a;
	}
	
	return res;
}

bool PickFile(HWND window, char* outFilename, int outFilenameMaxSize) {
	OPENFILENAMEA ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = window;
	ofn.lpstrFile = outFilename;
	ofn.nMaxFile = outFilenameMaxSize;
	ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	bool result = GetOpenFileNameA(&ofn);
	return result;
}

bool PickNewFile(HWND window, char* outFilename, int outFilenameMaxSize) {
	OPENFILENAMEA ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = window;
	ofn.lpstrFile = outFilename;
	ofn.nMaxFile = outFilenameMaxSize;
	ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_NOCHANGEDIR;

	bool result = GetOpenFileNameA(&ofn);
	return result;
}

// bool IsPrintableCharacter(int c) {
//   return !(c <= 31 || c == 127 || c == VK_BACK);
// }

bool IsLetter(char c) {
	return !(
		(c >= ' ' && c <= '/') ||
		(c >= ':' && c <= '@') ||
		(c >= '[' && c <= '`') ||
		(c >= '{' && c <= '`') ||
		c == '\t' || c == '\n'
	);
}

extern "C"
{
    #pragma function(memset)
    void *memset(void *dest, int c, size_t count)
    {
        char *bytes = (char *)dest;
        while (count--)
        {
            *bytes++ = (char)c;
        }
        return dest;
    }

    #pragma function(memcpy)
    void *memcpy(void *dest, const void *src, size_t count)
    {
        char *dest8 = (char *)dest;
        const char *src8 = (const char *)src;
        while (count--)
        {
            *dest8++ = *src8++;
        }
        return dest;
    }
}