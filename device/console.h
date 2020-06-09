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
 * @brief 在终端输出字符
 * 
 * @param c 
 * @param attr 
 */
void console_put_char(char c, uint8_t attr);

/**
 * @brief 获取屏幕上某个字符
 * 
 * @param pos
 */
uint32_t console_get_char_pos(uint16_t pos);

/**
 * @brief 在终端输出整数
 * 
 * @param x 
 * @param b 
 * @param attr 
 */
void console_put_int(int32_t x, uint32_t b, uint8_t attr);

/**
 * @brief 在终端特定位置输出字符
 * 
 * @param c 
 * @param attr 
 * @param pos_x 
 * @param pos_y 
 */
void console_put_char_in_pos(char c, uint8_t attr, uint8_t pos_x, uint8_t pos_y);

void console_put_str_in_pos(char *s, uint8_t attr, uint8_t pos_x, uint8_t pos_y);

void console_clear();