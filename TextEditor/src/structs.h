#pragma once
#include "tools.h"
#include "typedefs.h"

#define DYNAMIC_ARRAY_GROW_FACTOR 2
#define ARENA_GROW_FACTOR 2

void DebugWarningHandle(const char* msg);

struct vec2 {
  float x, y;
};

struct vec3 {
  float x, y, z;
};

struct ivec2 {
  int x, y;
};

struct ivec3 {
  int x, y, z;
};

#pragma region Memory Management
// Chained Arena - расширяется если не хватает места
// TDOO: free list
struct Arena {
	u8* mem = NULL;
	Arena* next = NULL;
	u32 size = 0, capacity = 0;

	void init(u32 capacity);
	void release();
  void clear();
	void* alloc(u32 size);
  void* realloc(void* oldMem, u32 oldSize, u32 newSize);
};

// struct TextBuffer {
//   char* text;
//   int capacity, size;
// };

#pragma endregion

// в коде необходимо писать, например template class Array<int>, если есть ошибки линковки
template<typename T>
struct Array {
  T* mem;
  int capacity, size;

  T& operator[](int i) {
    return mem[i];
  }

  void init(Arena* arena, int capacity) {
    size = 0;
    this->capacity = capacity;
    mem = (T*)arena->alloc(sizeof(T) * capacity);
  }

  void append(T value) {
    if (size < capacity)
      mem[size++] = value;
    else {
      DebugWarningHandle("ARRAY APPEND: not enough capacity");
    }
  }

  void insert(T value, int pos) {
    if (size < capacity) {
      for (int i = size; i > pos; i--)
        mem[i] = mem[i-1];
      mem[pos] = value;
      size++;
    }
    else {
      DebugWarningHandle("ARRAY INSERT: index out of bounds");
    }
  }

  void remove(int pos) {
    if (pos < size) {
      for (size_t i = pos; i < size - 1; i++)
        mem[i] = mem[i+1];
      size--;
    }
    else {
      DebugWarningHandle("ARRAY REMOVE: index out of bounds");
    }
  }
};

// TODO: убрать функции не работающие с ареной?
template<typename T>
struct DynamicArray {
  T* mem;
  int capacity, size;

  T& operator[](int i) {
    return mem[i];
  }

  void grow(Arena* arena, int newCapacity = 0) {
    if (newCapacity == 0 || newCapacity <= capacity)
      newCapacity = capacity * DYNAMIC_ARRAY_GROW_FACTOR;

    mem = (T*)arena->realloc(mem, size * sizeof(T), newCapacity * sizeof(T));

    capacity = newCapacity;
  }

  void reserve(Arena* arena, int newCapacity) {
    if (capacity >= newCapacity)
      return;

    mem = (T*)arena->realloc(mem, capacity, newCapacity);
    capacity = newCapacity;
  }

  void init(Arena* arena, int capacity) {
    size = 0;
    this->capacity = capacity;
    mem = (T*)arena->alloc(sizeof(T) * capacity);
  }

  void append(Arena* arena, T value) {
    if (size >= capacity) {
      grow(arena);
    }
    mem[size++] = value;
  }

  void insert(Arena* arena, T value, int pos) {
    if (size >= capacity) {
      grow(arena);
    }
    for (int i = size; i > pos; i--)
      mem[i] = mem[i-1];
    mem[pos] = value;
    size++;
  }

  void remove(int pos) {
    if (pos < size) {
      for (size_t i = pos; i < size - 1; i++)
        mem[i] = mem[i+1];
      size--;
    }
    else {
      DebugWarningHandle("DYNAMIC ARRAY REMOVE: index out of bounds");
    }
  }
};

template<typename T>
struct Stack {
  DynamicArray<T> items;

  void init(Arena* arena, int capacity) {
    items.init(arena, capacity);
  }

  void push(Arena* arena, T item) {
    items.append(arena, item);
  }

  T pop() {
    te_assert(items.size > 0)

    EditOperation op = items[items.size - 1];
    items.remove(items.size - 1);
    return op;
  }

  T peek() {
    te_assert(items.size > 0);
    return items[items.size - 1];
  }

  int size() {
    return items.size;
  }
};