#include "console.h"

#include "print.h"
#include "string.h"
#include "sync.h"
#include "thread.h"

static struct lock console_lock;

/**
 * @brief 初始化终端
 * 
 */
void console_init() {
    lock_init(&console_lock);
}

/**
 * @brief 获取终端
 * 
 */
void console_acquire() {
    lock_acquire(&console_lock);
}

/**
 * @brief 释放终端
 * 
 */
void console_release() {
    lock_release(&console_lock);
}

/**
 * @brief 在终端输出字符串
 * 
 * @param s 
 * @param attr 
 */
void console_put_str(char *s, uint8_t attr) {
    console_acquire();
    put_str(s, attr);
    console_release();
}

/**
 * @brief 在终端输出字符
 * 
 * @param c 
 * @param attr 
 */
void console_put_char(char c, uint8_t attr) {
    console_acquire();
    put_char(c, attr);
    console_release();
}

/**
 * @brief 获取屏幕上某个字符
 * 
 * @param pos
 */
uint32_t console_get_char_pos(uint16_t pos) {
    console_acquire();
    uint32_t c = get_char_pos(pos);
    console_release();
    return c;
}

/**
 * @brief 在终端输出整数
 * 
 * @param x 
 * @param b 
 * @param attr 
 */
void console_put_int(int32_t x, uint32_t b, uint8_t attr) {
    console_acquire();
    put_int(x, b, attr);
    console_release();
}

/**
 * @brief 在终端特定位置输出字符
 * 
 * @param c 
 * @param attr 
 * @param pos_x 
 * @param pos_y 
 */
void console_put_char_in_pos(char c, uint8_t attr, uint8_t pos_x, uint8_t pos_y) {
    console_acquire();
    put_char_in_pos(c, attr, pos_x, pos_y);
    console_release();
}

void console_put_str_in_pos(char *s, uint8_t attr, uint8_t pos_x, uint8_t pos_y) {
    console_acquire();
    uint32_t len = strlen(s);
    uint32_t pos = pos_x;
    pos = pos * 80 + pos_y;
    for (int i = 0; i < len; i++) {
        put_char_pos(s[i], attr, pos);
        pos++;
    }
    console_release();
}