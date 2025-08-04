#include "pieceTable.h"

#define DEFAULT_ADD_CAPACITY 2 * 1024
#define DEFAULT_NODES_CAPACITY 128
#define DEFAULT_TEXT_BUFFER_SIZE 1024

struct Text {
  PieceTableV2 pt;
  Arena arena;

  Array<char> textBuffer;
  char* filename;
  int id;
  int cursorIndex, cursorIndex2;
  bool windowFocused, textChanged;

  void init(
    int addCapacity = DEFAULT_ADD_CAPACITY, 
    int nodesCapacity = DEFAULT_NODES_CAPACITY, 
    int textBufferSize = DEFAULT_TEXT_BUFFER_SIZE
  );
  void free();
  bool removeChars(int count, int pos);
  void insertChars(char* c, int charCount, int pos);
  void insertChar(char c, int pos);
  void undo();
  void redo();

  int getLength();
  int toText();
  
  bool openFile(int addCapacity = 1024, int nodesCapacity = 1024);
  void saveFile();
};
