#include "print.h"

void put_int(int32_t x, uint32_t b, uint8_t attr) {
    if (x < 0) {
        put_char('-', attr);
        x = -x;
    }
    put_uint(x, b, attr);
}

void put_uint(uint32_t x, uint32_t b, uint8_t attr) {
    uint8_t s[20];
    int len = 0;
    if (x == 0) s[len++] = '0';
    while (x) {
        if ((x % b) >= 0 && (x % b) <= 9)
            s[len++] = x % b + '0';
        else
            s[len++] = x % b - 10 + 'A';
        x /= b;
    }
    s[len] = '\0';

    uint8_t tmp;
    for (int i = 0; i < len / 2; i++) {
        tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }

    put_str(s, attr);
}

void put_char_in_pos(uint8_t c, uint8_t attr, uint8_t pos_x, uint8_t pos_y) {
    put_char_pos(c, attr, pos_x * 80 + pos_y);
}

void set_cursor_in_pos(uint32_t pos_x, uint32_t pos_y) {
    set_cursor(pos_x * 80 + pos_y);
}

void clear() {
    set_cursor_in_pos(0, 0);
    for (int i = 0; i < 2000; i++) put_char(0, 0x07);
    set_cursor_in_pos(0, 0);
}

void debug_printf_s(uint8_t *s, uint8_t *c) {
    put_char('\n', 0x07);
    put_str(s, 0x07);
    put_str(c, 0x07);
    put_char('\n', 0x07);
}

void debug_printf_uint(uint8_t *s, uint32_t x, uint32_t b) {
    put_char('\n', 0x07);
    put_str(s, 0x07);
    put_uint(x, b, 0x07);
    put_char('\n', 0x07);
}
