#include "pieceTable.h"
#include "tools.h"

void PieceTableV2::init(char* origText, int addTextCapacity, int nodesCapacity) {
  original = origText;

  nodes.init(nodesCapacity);
  added.init(addTextCapacity);

  // первый узел с полным оригинальным текстом
  PTNode origNode;
  origNode.type = PTNodeType_Original;
  origNode.length = te_strlen(origText);
  origNode.start = 0;
  nodes.append(origNode);
}

void PieceTableV2::insert(char* text, int count, int index) {
  if (count <= 0)
    return;
  
  PTNode newNode;
  newNode.type = PTNodeType_Added;
  newNode.start = added.size;
  newNode.length = count;
  te_assert(newNode.length > 0);
  
  // TDOO: добавить в Array append сразу нескольких элементов
  for (size_t i = 0; i < count; i++) {
    added.append(text[i]);
  }

  // если вставка в начало
  if (index == 0) {
    nodes.insert(newNode, 0);
    return;
  }
  
  int textLen = 0;
  for (size_t i = 0; i < nodes.size; i++)
  {
    PTNode* curNode = &nodes[i];
    int start = curNode->start;
    int end = curNode->start + curNode->length;
    textLen += curNode->length;

    // вставка на дальней границе узла. добавляем узел после текущего 
    if (textLen == index) {
      if (curNode->type == PTNodeType_Added &&
       curNode->start + curNode->length == newNode.start) {
        curNode->length += newNode.length;
       }
      else {
        nodes.insert(newNode, i + 1);
       }
      break;
    }

    // встака не на границе узла
    if (textLen > index) {
      PTNode node1;
      node1.type = nodes[i].type;
      node1.start = nodes[i].start;
      // node1.length = index;
      node1.length = nodes[i].length - (textLen - index);

      PTNode node2;
      node2.type = nodes[i].type;
      // node2.start = index;
      node2.start = node1.start + node1.length;
      //node2.length = nodes[i].length - index;
      node2.length = textLen - index;
      
      te_assert(node1.length > 0 && node2.length > 0);

      nodes[i] = node1;
      nodes.insert(node2, i + 1);
      nodes.insert(newNode, i + 1);
      break;
    }
  }

    // TODO: убрать! толкьо для отладки!
    // char buf[4096];
    // toText(buf);
}

void PieceTableV2::remove(int remIndex, int remCount) {
  int textLen = 0;
  int remCountInitial = remCount;

  for (size_t i = 0; i < nodes.size; i++) 
  {
    int relativeIndex = remIndex - textLen; // индекс удаления относительно узла

    // удаление на левой границе узла
    if (relativeIndex == 0) {
      // полное удаление узла
      if (remCount == nodes[i].length) {
        nodes.remove(i);
        return;
      }
      // полное удаление узла и нужно продолжить удаление в след. узлах
      if (remCount > nodes[i].length) {
        remCount -= nodes[i].length;
        nodes.remove(i);
        i--;
        continue;
      }
      // неполное удаление узла
      else {
        nodes[i].start += remCount;
        nodes[i].length -= remCount;
        return;
      }
    }

    // удаление не на границе узла
    if (textLen + nodes[i].length > remIndex) {
      // удаление до правой границы узла
      if (remCount == nodes[i].length - relativeIndex) {
        nodes[i].length = remIndex - textLen;
        return;
      }
      // удаление до правой границы и продолжить удаление в след. узлах
      if (remCount > nodes[i].length - relativeIndex) {
        nodes[i].length = remIndex - textLen;
        remCount -= nodes[i].length - relativeIndex;
        continue;
      }
      // удаление в центре узла
      else {
        PTNode node1 = nodes[i];
        node1.length = relativeIndex;

        PTNode node2 = nodes[i];
        node2.start += relativeIndex + remCount;
        node2.length -= (relativeIndex + remCount);

        nodes[i] = node1;
        nodes.insert(node2, i + 1);
        
        return;
      }
    }

    textLen += nodes[i].length;
  }
}

int PieceTableV2::getLength()
{
  int len = 0;
  for (size_t i = 0; i < nodes.size; i++)
    len += nodes[i].length;
  return len;
}

int PieceTableV2::toText(char* buf) {
  char* bufSlider = buf;
  for (size_t i = 0; i < nodes.size; i++)
  {
    PTNode* node = &nodes[i];

    char* source = 0;
    if (node->type == PTNodeType_Original)
      source = original;
    else if (node->type == PTNodeType_Added)
      source = added.mem;
    else 
      ErrorHandle();

    te_memcpy(bufSlider, source + node->start, node->length);
    bufSlider += node->length;
  }

  buf[bufSlider-buf] = '\0';

  return bufSlider - buf; // кол-во записанных элементов
}