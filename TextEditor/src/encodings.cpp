#include "encodings.h"
#include "tools.h"

CharTypeU8 getCharTypeU8(unsigned char c) {
  if      (Is1ByteU8(c)) return Char1Byte;
  else if (Is2ByteU8(c)) return Char2Byte;
  else if (Is3ByteU8(c)) return Char3Byte;
  else if (Is4ByteU8(c)) return Char4Byte;
  
  // in middle of multi-byte characher
  else if (IsMiddleByteU8(c)) return CharMiddle;
  else return CharNONE;
}

int getCharLenU8(unsigned char c) {
  if      (Is1ByteU8(c)) return 1;
  else if (Is2ByteU8(c)) return 2;
  else if (Is3ByteU8(c)) return 3;
  else if (Is4ByteU8(c)) return 4;
  
  // in middle of multi-byte characher
  else if (IsMiddleByteU8(c)) return 1;
  else return 0;
}

const char* findCharStartU8(const char* s) {
  CharTypeU8 type = getCharTypeU8(*s);
  if (type == CharMiddle) {
    int i = -1;
    while (IsMiddleByteU8(s[i])) {
      i--;
    }
    return &s[i];
  }
  else {
    return s;
  }
}

int countCharsU8(const char* s) {
  int i = 0;
  int count = 0;
  while (s[i] != '\0') {
    unsigned char c = s[i];
    switch (getCharTypeU8(c))
    {
    case Char1Byte:
      count++; i += 1; break;
    case Char2Byte:
      count++; i += 2; break;
    case Char3Byte:
      count++; i += 3; break;
    case Char4Byte:
      count++; i += 4; break;
    case CharNONE:
      i += 1; break;
    case CharMiddle:
      i += 1; break;
    default:
      ErrorHandle();
    }
  }
  return count;
}