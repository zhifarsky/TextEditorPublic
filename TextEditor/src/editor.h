#pragma once
#include <windows.h>
#include "text.h"

#define DEFAULT_TABS_CAPACITY 32

struct RenderInfo {
  HDC deviceContext;
  HGLRC oglContext;
  HWND window;
};

struct Editor {
  DynamicArray<Text> textTabs;
  DynamicArray<bool> isTextOpen;
  RenderInfo renderInfo;
  int currentTextTab;
  bool showCommandsWindow;
  
  void init(int tabsCapacity = DEFAULT_TABS_CAPACITY);

  void cursorMoveLeft();
  void cursorMoveRight();
  void cursorMoveUp();
  void cursorMoveDown();

  void ProcessHotkey(HWND window);
  void ProcessKeydown(unsigned int key);
  void ProcessChar(unsigned int key);

  Text& getCurrentTextTab();
  void AddTextTab();
};

// глобальноые состояние программы
extern Editor g_editor;

// enum te_hotkeys {
//   hk_copy,
//   hk_paste,
//   hk_commands,
//   hk_COUNT
// };



int getUTF8ByteCount (unsigned int c);