#include "stdio.h"

#include "console.h"
#include "global.h"
#include "interrupt.h"
#include "print.h"
#include "string.h"
#include "syscall.h"

void itoa(char* s, uint32_t x, uint32_t b) {
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
}

uint32_t vsprintf(char* s, const char* format, va_list args) {
    int len = 0;
    uint32_t arg_int;
    char* arg_s;
    char arg_c;
    for (int i = 0; i < strlen(format); i++) {
        if (format[i] == '%') {
            switch (format[i + 1]) {
                case 'x':
                    arg_int = va_arg(args, uint32_t);
                    itoa(&s[len], arg_int, 16);
                    len = strlen(s);
                    break;
                case 'd':
                    arg_int = va_arg(args, uint32_t);
                    itoa(&s[len], arg_int, 10);
                    len = strlen(s);
                    break;
                case 's':
                    arg_s = va_arg(args, char*);
                    strcpy(&s[len], arg_s);
                    len = strlen(s);
                    break;
                case 'c':
                    arg_c = va_arg(args, char);
                    s[len++] = arg_c;
                default:
                    break;
            }
            i++;
        } else {
            s[len++] = format[i];
        }
    }
    s[len] = '\0';
    return strlen(s);
}

uint32_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1000] = {0};
    vsprintf(buf, format, args);
    va_end(args);
    return write(buf, 0x07);
}


uint32_t sprintf(char* buf, const char* format, ...) {
   va_list args;
   uint32_t retval;
   va_start(args, format);
   retval = vsprintf(buf, format, args);
   va_end(args);
   return retval;
}


char putchar(char c) {
    char buf[2] = {0};
    buf[0] = c;
    write(buf, 0x07);
    return c;
}

uint32_t puts(char* s) {
    return write(s, 0x07);
}

uint32_t s_to_uint(char* value) {
    uint32_t x = 0;
    for (int i = 0; i < strlen(value); i++) {
        if (value[i] >= '0' && value[i] <= '9') {
            x = x * 10 + value[i] - '0';
        } else if (value[i] >= 'a' && value[i] <= 'z') {
            x = x * 10 + value[i] - 'a' + 10;
        } else if (value[i] >= 'A' && value[i] <= 'Z') {
            x = x * 10 + value[i] - 'A' + 10;
        }
    }
    return x;
}

uint32_t vscanf(const char* format, va_list args) {
    int len = 0;
    uint32_t* arg_int;
    char* arg_s;
    char value[1000];
    for (int i = 0; i < strlen(format); i++) {
        if (format[i] == '%') {
            switch (format[i + 1]) {
                case 'd':
                case 'x':
                    arg_int = va_arg(args, uint32_t*);
                    read(value);
                    *arg_int = s_to_uint(value);
                    break;
                case 'c':
                case 's':
                    arg_s = va_arg(args, char*);
                    read(value);
                    strcpy(arg_s, value);
                default:
                    break;
            }
            i++;
        }
    }
}

void scanf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vscanf(format, args);
    va_end(args);
}

char getchar() {
    return read_char();
}

void gets(char* s) {
    read(s);
}

/**
 * @brief 内核使用的printf
 * 
 * @param format 
 * @param ... 
 */
void kprintf(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char buf[1024] = {0};
   vsprintf(buf, format, args);
   va_end(args);
//    console_put_str(buf, 0x07);
}