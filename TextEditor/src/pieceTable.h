#pragma once
#include "structs.h"

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
  char* original;

  void init(char* origText, int addTextCapacity, int nodesCapacity);
  void insert(char* text, int count, int index);
  void remove(int index, int count);
  int getLength();
  int toText(char* buf);
};