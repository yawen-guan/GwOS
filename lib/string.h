#ifndef __LIB_STRING_H
#define __LIB_STRING_H

#include "stdint.h"

void memset(void *dst, uint8_t data, uint32_t size);

void memcpy(void *dst, const void *src, uint32_t size);

char* strcpy(char *dst, const char *src);

uint32_t strlen(const char *str);

int8_t strcmp(const char *a, const char *b);


#endif