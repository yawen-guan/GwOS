// #ifndef __KERBOARD_H
// #define __KERBOARD_H

#pragma once

#include "global.h"
#include "ioqueue.h"

extern bool ouch;

/**
 * @brief 键盘中断处理函数
 */
static void intr_keyboard_handler();

/**
 * @brief 键盘初始化
 * 
 */
void keyboard_init();

/**
 * @brief 从键盘缓冲区读入一个字符
 * 
 * @return char 
 */
char keyboard_getchar();