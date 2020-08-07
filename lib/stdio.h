#pragma once

#include "global.h"
#include "stdint.h"

typedef char* va_list;

#define va_start(ap, v) ap = (va_list)&v      // 把ap指向第一个固定参数v
#define va_arg(ap, type) *((type*)(ap += 4))  // ap指向下一个参数,返回参数值
#define va_end(ap) ap = NULL                  // 清除a

uint32_t printf(const char* format, ...);

uint32_t sprintf(char* buf, const char* format, ...);

char putchar(char c);

uint32_t puts(char* s);

void scanf(const char* format, ...);

char getchar();

void gets(char* s);

/**
 * @brief 内核使用的printf
 * 
 * @param format 
 * @param ... 
 */
void kprintf(const char* format, ...);