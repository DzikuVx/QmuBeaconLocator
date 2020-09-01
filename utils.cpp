#include "utils.h"

void int32ToBuf(uint8_t buffer[], uint8_t index, long value) {
    buffer[index] = 

    buffer[index] = value & 0xFF;
    buffer[index + 1] = (value >> 8) & 0xFF;
    buffer[index + 2] = (value >> 16) & 0xFF;
    buffer[index + 3] = (value >> 24) & 0xFF;
}