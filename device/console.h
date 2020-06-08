#pragma once

#include "stdint.h"

/**
 * @brief 初始化终端
 * 
 */
void console_init();

/**
 * @brief 获取终端
 * 
 */
void console_acquire();

/**
 * @brief 释放终端
 * 
 */
void console_release();

/**
 * @brief 在终端输出字符串
 * 
 * @param s 
 * @param attr 
 */
void console_put_str(char *s, uint8_t attr);
/**
 * @brief 在终端输出整数
 * 
 * @param x 
 * @param b 
 * @param attr 
 */
void console_put_int(int32_t x, uint32_t b, uint8_t attr);