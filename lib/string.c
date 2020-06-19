#include "string.h"

#include "debug.h"
#include "global.h"

/**
 * 将dst起始的size个字节置为value
 */
void memset(void *dst, uint8_t data, uint32_t size) {
    uint8_t *u8dst = (uint8_t *)dst;
    while (size) {
        *u8dst++ = data;
        size--;
    }
}

/**
 * 将src起始的size个字节复制到dst
 */
void memcpy(void *dst, const void *src, uint32_t size) {
    uint8_t *u8dst = dst;
    const uint8_t *u8src = src;
    while (size) {
        *u8dst++ = *u8src++;
        size--;
    }
}

char *strcpy(char *dst, const char *src) {
    char *st = dst;
    while (*dst++ = *src++)
        ;
    return st;
}

uint32_t strlen(const char *str) {
    const char *p = str;
    while (*p++)
        ;
    return (p - str - 1);
}

int8_t strcmp(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
}