#pragma once
#include "structs.h"

struct EditOperation;

enum PTNodeType : char {
  PTNodeType_Original,
  PTNodeType_Added
};

struct PTNode {
  int start, length;
  PTNodeType type;
};

struct PieceTableV2 {
  DynamicArray<char> added;
  DynamicArray<PTNode> nodes;
  Stack<EditOperation> undoStack, redoStack;
  char* original;

  void init(Arena* arena, char* origText, int addTextCapacity, int nodesCapacity);
  void insertText(Arena* arena, char* text, int count, int index, bool writeHistory = true);
  void insertNode(Arena* arena, PTNode node, int index, bool writeHistory = true);
  void remove(Arena* arena, int index, int count, bool writeHistory = true);
  int getLength();
  int toText(char* buf);
};

enum EditOperationType : char {
  EditOperationInsert,
  EditOperationDelete
};

struct EditOperation {
  PTNode deletedNode;
  int start, length;
  EditOperationType type;

  EditOperation();
  EditOperation(EditOperationType type, int start, int length);
};
