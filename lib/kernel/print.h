// #ifndef __LIB_KERNEL_PRINT_H
// #define __LIB_KERNEL_PRINT_H

#pragma once

#include "stdint.h"

void put_char(uint8_t c, uint8_t attr);

void put_char_pos(uint8_t c, uint8_t attr, uint16_t pos);
void put_char_in_pos(uint8_t c, uint8_t attr, uint8_t pos_x, uint8_t pos_y);

void put_str(uint8_t *s, uint8_t attr);
void put_int(int32_t x, uint32_t b, uint8_t attr);  //b进制的数据x
void put_uint(uint32_t x, uint32_t b, uint8_t attr);

void set_cursor(uint32_t pos);
void set_cursor_in_pos(uint32_t pos_x, uint32_t pos_y);

void debug_printf_s(uint8_t *s, uint8_t *c);
void debug_printf_uint(uint8_t *s, uint32_t x);

// #endif
