#include "tools.h"

void te_memcpy(void* destination, void* source, u64 size) {
    u8 *dest = (u8*)destination;
    u8 *src = (u8*)source;

    while (size--) {
        *dest++ = *src++;
    }
}
