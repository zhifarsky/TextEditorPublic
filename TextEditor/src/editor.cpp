#include <windows.h>
#include "editor.h"
#include "tools.h"
#include "encodings.h"

void Editor::init(int tabsCapacity) {
  currentTextTab = 0;
  textTabs.init(tabsCapacity);
  isTextOpen.init(tabsCapacity);
}

Text& Editor::getCurrentTextTab() {
  return textTabs[currentTextTab];
}

void Editor::AddTextTab() {
  Text text;
  text.init();
  textTabs.append(text);
  isTextOpen.append(true);
}

void Editor::CloseTextTab(int tabIndex) {
  textTabs[currentTextTab].free();
  isTextOpen.remove(tabIndex);
  textTabs.remove(tabIndex);
}

void Editor::cursorMoveLeft() {
  Text& text = getCurrentTextTab();
  
  // на один символ
  if (!te_IsKeyDown(VK_CONTROL)) {
    // if (text.cursorIndex2 < 1) {
    //   text.cursorIndex = text.cursorIndex2;
    //   return;
    // }

    int offset = findOffsetToCharStartU8(&text.textBuffer[max(text.cursorIndex2 - 1, 0)]);
    if (!te_IsKeyDown(VK_SHIFT)) // перемещение курсора
      text.cursorIndex = text.cursorIndex2 = max(text.cursorIndex + offset - 1, 0);
    else // выделение текста
      text.cursorIndex2 = max(text.cursorIndex2 + offset - 1, 0);
  }
  // пропустить все буквы
  else {
    int i = te_IsKeyDown(VK_SHIFT) ? text.cursorIndex2 - 1 : text.cursorIndex - 1;
    for (; i >= 0; i--)
    {
      char c = text.textBuffer[i];
      if (!IsLetter(c)) {
        break;
      }
    }
    if (!te_IsKeyDown(VK_SHIFT))
      text.cursorIndex = max(i, 0);
    text.cursorIndex2 = max(i, 0);
  }
}

void Editor::cursorMoveRight() {
  Text& text = getCurrentTextTab();
  
  // if (text.cursorIndex < text.textBuffer.size) {
    // на один символ
    if (!te_IsKeyDown(VK_CONTROL)) {
      int offset = getCharLenU8(text.textBuffer[text.cursorIndex]);
      if (!te_IsKeyDown(VK_SHIFT)) {
        text.cursorIndex = min(text.cursorIndex + offset, text.textBuffer.size);
        text.cursorIndex2 = text.cursorIndex;
      }
      // выделение текста
      else {
        text.cursorIndex2 = min(text.cursorIndex2 + offset, text.textBuffer.size);
      }
    }
    // пропустить все буквы
    else {
      int i = te_IsKeyDown(VK_SHIFT) ? text.cursorIndex2 + 1 : text.cursorIndex + 1;
      for (; i < text.textBuffer.size; i++)
      {
        char c = text.textBuffer[i];
        if (!IsLetter(c)) {
          break;
        }
      }
      if (!te_IsKeyDown(VK_SHIFT))
        text.cursorIndex = min(i, text.textBuffer.size);
      text.cursorIndex2 = min(i, text.textBuffer.size);
    }
  // }
}

// переход на строку вверх
void Editor::cursorMoveUp()
{
  Text& text = getCurrentTextTab();
  
  // находим начало и длину текущей строки
  int lineLen = 0;
  int i = text.cursorIndex;
  for (; i > 0; i--)
  {
    if (text.textBuffer[i-1] == '\n') {
      lineLen = text.cursorIndex - i;
      break;
    }
  }
  
  // находим начало и длину предыдущей строки
  int prevLineStart = 0;
  for (int j = i - 1; j > 0; j--)
  {
    if (text.textBuffer[j - 1] == '\n') {
      prevLineStart = j;
      break;
    }
  }
  int prevLineLen = max(i - 1 - prevLineStart, 0);

  text.cursorIndex = text.cursorIndex2 = prevLineStart + min(lineLen, prevLineLen);
}

// переход на строку вниз
void Editor::cursorMoveDown()
{
  Text& text = getCurrentTextTab();
  
  // находим начало и длину текущей строки
  int lineLen = 0;
  int i = text.cursorIndex;
  for (; i > 0; i--)
  {
    if (text.textBuffer[i-1] == '\n') {
      break;
    }
  }
  lineLen = text.cursorIndex - i;

  int nextLineStart = 0;
  // находим начало следующей строки
  for (int j = text.cursorIndex; j < text.textBuffer.size; j++) {
    if (text.textBuffer[j] == '\n') {
      nextLineStart = j + 1;
      break;
    }
  }

  text.cursorIndex = text.cursorIndex2 = min(nextLineStart + lineLen, text.textBuffer.size);
}

void CopyToClipboard(const char* text, HWND window) {
    // Открываем буфер обмена
    if (!OpenClipboard(window)) {
      OutputDebugStringA("Error on opening clipboard\n");
      WarningHandle("Error on opening clipboard");
      return;
    }

    // Очищаем буфер обмена
    if (!EmptyClipboard()) {
      OutputDebugStringA("Error on clearing clipboard\n");
      CloseClipboard();
      return;
    }

    int textLen = te_strlen(text) + 1;

    // Выделяем память для текста
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, textLen);
    if (hMem == NULL) {
      OutputDebugStringA("Error on mem alloc\n");
      CloseClipboard();
      return;
    }

    // Копируем текст в выделенную память
    char* pMem = (char*)GlobalLock(hMem);
    // strcpy(pMem, text);
    te_memcpy(pMem, (char*)text, textLen); // TDOO: заменить на strcpy
    GlobalUnlock(hMem);

    // Устанавливаем данные в буфер обмена
    if (!SetClipboardData(CF_TEXT, hMem)) {
        OutputDebugStringA("Error on setting clipboard data\n");
        GlobalFree(hMem);
        CloseClipboard();
        return;
    }

    // Закрываем буфер обмена (память освобождается системой)
    CloseClipboard();
}

char* GetFromClipboard(HWND window) {
  // Откр ываем буфер обмена
  if (!OpenClipboard(window)) {
    OutputDebugStringA("Error on opening clipboard\n");
    WarningHandle("Error on opening clipboard");
    return NULL;
  }

    // Проверяем, есть ли текст в формате ANSI
  if (!IsClipboardFormatAvailable(CF_TEXT)) {
      // printf("В буфере нет текста (ANSI)\n");
      CloseClipboard();
      return NULL;
  }

  // Получаем данные из буфера
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == NULL) {
      // printf("Ошибка: не удалось получить данные\n");
      CloseClipboard();
      return NULL;
  }

  // Блокируем память и копируем текст
  char* pszText = (char*)GlobalLock(hData);
  if (pszText == NULL) {
      // printf("Ошибка: не удалось заблокировать память\n");
      CloseClipboard();
      return NULL;
  }

  // Копируем в новую строку (чтобы не зависеть от системного буфера)
  int textLen = te_strlen(pszText) + 1; 
  char* clipboardText = (char*)te_malloc(textLen);
  te_memcpy(clipboardText, pszText, textLen);

  // Разблокируем и закрываем буфер
  GlobalUnlock(hData);
  CloseClipboard();
  
  return clipboardText;
}

bool hotkey(bool isPressed, bool* wasPressed) {
  if (isPressed) {
    if (!*wasPressed) {
      *wasPressed = true;
      return true;
    } else {
      return false;
    }
  } else {
    *wasPressed = false;
    return false;
  }

}

void Editor::ProcessHotkey(HWND window) {
  if (window != GetActiveWindow())
    return;

  Text& text = getCurrentTextTab();
  
  // копировать
  static bool copy = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('C'), &copy)) {
    if (text.cursorIndex != text.cursorIndex2) {
      int strStart = min(text.cursorIndex, text.cursorIndex2);
      int strEnd = max(text.cursorIndex, text.cursorIndex2);

      int textLen = text.getLength() + 1;
      char* buf = (char*)te_malloc(textLen);
      text.pt.toText(buf);
      buf[strEnd] = '\0';
      
      CopyToClipboard(buf + strStart, window);
      te_free(buf);
    }
  }

  // вставить
  static bool paste = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('V'), &paste)) {
    char* mem = GetFromClipboard(window);
    int len = te_strlen(mem);

    text.insertChars(mem, len, text.cursorIndex);
    te_free(mem);

    text.cursorIndex += len;
    text.cursorIndex2 = text.cursorIndex;
  }

  // выбрать все
  static bool selectAll = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('A'), &selectAll)) {
    text.cursorIndex = 0;
    text.cursorIndex2 = text.textBuffer.size;
  }

  // новая вкладка
  static bool newText = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && (te_IsKeyDown('N') || te_IsKeyDown('T')), &newText)) {
    AddTextTab();
  }

  // закрыть текущую вкладку
  static bool closeText = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('W'), &closeText)) {
    if (currentTextTab >= 0 && currentTextTab < textTabs.size) {
      CloseTextTab(currentTextTab);
    }
  }

  static bool undo = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('Z'), &undo)) {
    getCurrentTextTab().undo();
  }
  // if (te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('Z')) {
  //   getCurrentTextTab().undo();
  // }


  static bool redo = false;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown('Y'), &redo)) {
    getCurrentTextTab().redo();
  }

  // окно команд
  static bool commands = true;
  if (hotkey(te_IsKeyDown(VK_CONTROL) && te_IsKeyDown(VK_SHIFT) && te_IsKeyDown('P'), &commands)) {
    showCommandsWindow = !showCommandsWindow;
  }
}

void Editor::ProcessKeydown(unsigned int key) {
  // if (!textWindowFocused) return;
  
  Text& text = getCurrentTextTab();

  switch (key)
  {
    // backspace
    case VK_BACK: {
      
      // удаление выеделения
      if (text.cursorIndex != text.cursorIndex2) {
          int start = min(text.cursorIndex, text.cursorIndex2);
          int len = max(text.cursorIndex, text.cursorIndex2) - start;

          if (start >= 0) {
            if (text.removeChars(len, start)) {
              text.cursorIndex = start;
              text.cursorIndex2 = start;
            }
          }
      }
      // удаление 1 символа
      else if (text.cursorIndex > 0) {
        const char* charStart = findCharStartU8(&text.textBuffer[text.cursorIndex - 1]);
        int charIndex = charStart - text.textBuffer.mem;
        int charLen = getCharLenU8(*charStart);
        if (text.removeChars(charLen, charIndex)) {
          // text.cursorIndex--;
          // text.cursorIndex2--;
          text.cursorIndex = text.cursorIndex2 = charIndex;
        }
      }
    } break;

    case VK_DELETE: {
      // удаление выеделения
      if (text.cursorIndex != text.cursorIndex2) {
        int start = min(text.cursorIndex, text.cursorIndex2);
        int len = max(text.cursorIndex, text.cursorIndex2) - start;
        if (start >= 0) {
          if (text.removeChars(len, start)) {
            text.cursorIndex = start;
            text.cursorIndex2 = start;
          }
        }
      }
      // удаление 1 символа
      else if (text.cursorIndex >= 0) {
        int charLen = getCharLenU8(text.textBuffer[text.cursorIndex]);
        if (text.removeChars(charLen, text.cursorIndex)) {
        }
      }
    } break;
    
    // <-
    case VK_LEFT: {
      cursorMoveLeft();
    } break;
    
    // ->
    case VK_RIGHT: {
      cursorMoveRight();
    }	break;

    case VK_UP: {
      cursorMoveUp();
    } break;

    case VK_DOWN: {
      cursorMoveDown();
    } break;

    // enter
    case VK_RETURN: {
      // text->buffer[text->bufferSize++] = '\n';
      text.insertChar('\n', text.cursorIndex);
      text.cursorIndex++;
      text.cursorIndex2++;
    } break;
  }
}

void Editor::ProcessChar(unsigned int key) {
  if (textTabs.size < 1) return;
  
  Text& text = getCurrentTextTab();

  // Пропускаем управляющие символы (Backspace уже обрабатывается отдельно)
  // if (key <= 31 || key == 127 || key == VK_BACK) return;

  wchar_t wideChar = (wchar_t)key;

  // Проверяем, является ли символ печатаемым
  if (wideChar < 32 || (wideChar >= 127 && wideChar < 160)) {
    return;
  }

  // TODO: заменить winapi функцию на собственную реализацию 
  char utf8Buffer[5] = {0};
  int bytesWritten = WideCharToMultiByte(CP_UTF8, 0, &wideChar, 1, utf8Buffer, sizeof(utf8Buffer) - 1, NULL, NULL);

  text.insertChars(utf8Buffer, bytesWritten, text.cursorIndex);
  text.cursorIndex += bytesWritten;
  text.cursorIndex2 += bytesWritten;
}
