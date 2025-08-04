#include <windows.h>
#include "text.h"
#include "tools.h"

static int g_currentTextID = 0;

void Text::init(int addCapacity, int nodesCapacity, int textBufferSize) {
  id = g_currentTextID++; 
  cursorIndex = cursorIndex2 = 0;
  filename = 0;
  windowFocused = false; 
  textChanged = true;
  
  arena.init(16 * 1024);
  pt.init(&arena, "", addCapacity, nodesCapacity);

  textBuffer.init(&arena, textBufferSize);
}

void Text::free() {
  arena.release();
}

void Text::insertChar(char c, int pos) {
  textChanged = true;
  pt.insertText(&arena, &c, 1, pos);
}


void Text::insertChars(char* c, int charCount, int pos) {
  textChanged = true;
  pt.insertText(&arena, c, charCount, pos);
}

bool Text::removeChars(int count, int pos) {
  if (pos < 0 || count <= 0)
    return false;

  textChanged = true;
  pt.remove(&arena, pos, count);
  return true;
}

void Text::undo() {
  if (pt.undoStack.size() < 1)
    return;

  EditOperation op = pt.undoStack.pop();
  pt.redoStack.push(&arena, op);
  
  switch (op.type) {
    // если нужно отменить вставку - делаем удаление
    case EditOperationInsert: {
      pt.remove(&arena, op.start, op.length, false);
      cursorIndex = cursorIndex2 = op.start;
      textChanged = true;
    } break;

  // если нужно отменить удаление - делаем вставку
    case EditOperationDelete: {
      pt.insertNode(&arena, op.deletedNode, op.start, false);
      textChanged = true;
    } break;

    default: 
      OutputDebugStringA("Unknown edit operation\n");
  }
}

void Text::redo() {
  // if (pt.redoStack.size() < 1)
  //   return;

  // EditOperation op = pt.redoStack.pop();
  // pt.undoStack.push(&arena, op);
  
  // switch (op.type) {
  // // если нужно вернуть удаление
  //   case EditOperationDelete: {
  //     pt.remove(&arena, op.start, op.length, false);
  //     cursorIndex = cursorIndex2 = op.start;
  //     textChanged = true;
  //   } break;

  //   // если нужно вернуть вставку
  //   case EditOperationInsert: {
  //     pt.insertNode(&arena, op.deletedNode, op.start, false);
  //     textChanged = true;
  //   } break;

  //   default: 
  //     OutputDebugStringA("Unknown edit operation\n");
  // }
}

int Text::getLength() {
  return pt.getLength();
}

int Text::toText() {
  if (textChanged) {
    int textLen = getLength();
    if (textLen > textBuffer.size) {
      //textBuffer.free();
      textBuffer.init(&arena, textLen * 2);
    }
    textChanged = false;
    return pt.toText(textBuffer.mem);
  }
  else {
    return textBuffer.size;
  }
}

bool Text::openFile(int addCapacity, int nodesCapacity) {
  if (!filename)
    filename = (char*)te_malloc(256);

  if (!PickFile(NULL, filename, 256)) {
    return false;
  }

  HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
  if (!hFile) {
    WarningHandle("Error on opening file");
    return false;
  }
  
  DWORD fileSize = GetFileSize(hFile, NULL);
  char *buffer = (char*) te_malloc(fileSize + 1);

  DWORD bytesRead;
  if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
    pt.init(&arena, buffer, addCapacity, nodesCapacity);
    textChanged = true;
    CloseHandle(hFile);
    return true;
  }
  else {
    te_free(buffer);
    CloseHandle(hFile);
    WarningHandle("Error on opening file");
    return false;
  }
}

void Text::saveFile() {
  // save as new
  if (filename == NULL) {
    filename = (char*)te_malloc(256);

    if (PickNewFile(NULL, filename, 256)) {
      int bufLen = getLength();
      char* buf = (char*)te_malloc(bufLen + 1);
      
      pt.toText(buf);
      buf[bufLen] = '\n';
      
      SaveToFileWin32(filename, buf, bufLen + 1);
    }
  }
  // save existing
  else {
    int bufLen = getLength();
    char* buf = (char*)te_malloc(bufLen + 1);
    
    pt.toText(buf);
    buf[bufLen] = '\n';

    SaveToFileWin32(filename, buf, bufLen + 1);
  }

}
