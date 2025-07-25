enum CharTypeU8 {
  Char1Byte,
  Char2Byte,
  Char3Byte,
  Char4Byte,
  CharMiddle, // midlle of multi-byte character
  CharNONE,
};

#define Is1ByteU8(c) (c & 0b10000000) == 0 
#define Is2ByteU8(c) (c & 0b11100000) == 0b11000000
#define Is3ByteU8(c) (c & 0b11110000) == 0b11100000
#define Is4ByteU8(c) (c & 0b11111000) == 0b11110000
#define IsMiddleByteU8(c) (c & 0b11000000) == 0b10000000

CharTypeU8 getCharTypeU8(unsigned char c);
int getCharLenU8(unsigned char c);
const char *findCharStartU8(const char *s);
int countCharsU8(const char *s);