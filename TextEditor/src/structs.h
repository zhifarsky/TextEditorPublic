#pragma once
#include "tools.h"

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

// struct DynamicArray {
//   void* mem;
//   int capacity;
//   int size;
// };

// void dynamicArrayInit(DynamicArray* dynamicArray) {}

struct TextBuffer {
  char* text;
  int capacity, size;
};

// в коде необходимо писать, например template class Array<int>, если есть ошибки линковки
template<typename T>
struct Array {
  T* mem;
  int capacity, size;

  T& operator[](int i) {
    return mem[i];
  }

  void init(int capacity) {
    size = 0;
    this->capacity = capacity;
    mem = (T*)te_malloc(sizeof(T) * capacity);
  }

  void free() {
    te_free(mem);
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

#define DYNAMIC_ARRAY_GROW_FACTOR 2

template<typename T>
struct DynamicArray {
  T* mem;
  int capacity, size;

  T& operator[](int i) {
    return mem[i];
  }

  void grow() {
    int test = sizeof(T);
    char buf[64];
    buf[0] = '0' + test;
    buf[1] = '\0';
    OutputDebugStringA(buf);
    mem = (T*)te_realloc(mem, size * sizeof(T), capacity * DYNAMIC_ARRAY_GROW_FACTOR * sizeof(T));
    capacity *= DYNAMIC_ARRAY_GROW_FACTOR;
  }

  void reserve(int newCapacity) {
    if (capacity >= newCapacity)
      return;

    mem = (T*)te_realloc(mem, capacity, newCapacity);
    capacity = newCapacity;
  }

  void init(int capacity) {
    size = 0;
    this->capacity = capacity;
    mem = (T*)te_malloc(sizeof(T) * capacity);
  }

  void free() {
    te_free(mem);
  }

  void append(T value) {
    if (size >= capacity) {
      grow();
    }
    mem[size++] = value;
  }

  void insert(T value, int pos) {
    if (size >= capacity) {
      grow();
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

  void init(int capacity) {
    items.init(capacity);
  }
  
  void free() {
    items.free();
  }

  void push(T item) {
    items.append(item);
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