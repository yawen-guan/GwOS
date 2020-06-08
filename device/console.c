#include "console.h"

#include "print.h"
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