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
  
  pt.init("", addCapacity, nodesCapacity);
  undoStack.init(DEFAULT_HISTORY_STACK_SIZE);
  redoStack.init(DEFAULT_HISTORY_STACK_SIZE);
  textBuffer.init(textBufferSize);
}

void Text::free() {
  pt.added.free();
  pt.nodes.free();
  undoStack.free();
  redoStack.free();
  textBuffer.free();
}

void Text::insertChar(char c, int pos) {
  textChanged = true;
  pt.insert(&c, 1, pos);
}


void Text::insertChars(char* c, int charCount, int pos) {
  textChanged = true;
  // char* insertedText = pt.added.mem + pt.added.size;
  pt.insert(c, charCount, pos);

  EditOperation op;
  op.type = EditOperationInsert;
  op.start = pos;
  op.length = charCount;
  // op.text = insertedText;
  undoStack.push(op);
}

bool Text::removeChars(int count, int pos) {
  if (pos < 0 || count <= 0)
    return false;

  textChanged = true;
  pt.remove(pos, count);
  
  EditOperation op;
  op.type = EditOperationDelete;
  op.start = pos;
  op.length = count;
  undoStack.push(op);

  return true;
}

void Text::undo() {
  if (undoStack.size() < 1)
    return;

  EditOperation op = undoStack.pop();
  redoStack.push(op);
  
  switch (op.type) {
    // если нужно отменить вставку - делаем удаление
    case EditOperationInsert: {
      pt.remove(op.start, op.length);
      cursorIndex = cursorIndex2 = op.start;
      textChanged = true;
    } break;

  // если нужно отменить удаление - делаем вставку
    case EditOperationDelete: {
      // pt.insert(op.text, op.length, op.start);
      // textChanged = true;
    } break;

    default: 
      OutputDebugStringA("Unknown edit operation\n");
  }
}

void Text::redo() {

}

int Text::getLength() {
  return pt.getLength();
}

int Text::toText() {
  if (textChanged) {
    int textLen = getLength();
    if (textLen > textBuffer.size) {
      textBuffer.free();
      textBuffer.init(textLen * 2);
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
    pt.init(buffer, addCapacity, nodesCapacity);
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
