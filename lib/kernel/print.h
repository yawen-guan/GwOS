#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H

#include "stdint.h"

void put_char(uint8_t c, uint8_t attr);
void put_char_pos(uint8_t c, uint8_t attr, uint16_t pos);
void put_char_in_pos(uint8_t c, uint8_t attr, uint8_t pos_x, uint8_t pos_y);
void put_str(uint8_t *s, uint8_t attr);
void put_int(int32_t x, uint32_t b, uint8_t attr); //b进制的数据x
void put_uint(uint32_t x, uint32_t b, uint8_t attr);

#endif

