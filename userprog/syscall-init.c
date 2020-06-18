#include "syscall-init.h"

#include "console.h"
#include "global.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"
#include "syscall.h"
#include "thread.h"

#define syscall_cnt 32

void* syscall_table[syscall_cnt];

uint32_t sys_get_pid() {
    return running_thread()->pid;
}

uint32_t sys_write(char* s, uint32_t attr) {
    console_put_str(s, attr);
    return strlen(s);
}

void sys_read(char* s) {
    uint32_t len = 0;
    char c;
    while (true) {
        c = ioq_getchar(&keyboard_ioq);

        if (c == '\n' || c == '\r' || c == ' ') break;
        s[len++] = c;
    }
    s[len] = '\0';
}

char sys_read_char() {
    return ioq_getchar(&keyboard_ioq);
}

void sys_write_in_pos(char* s, uint32_t attr, uint32_t pos) {
    console_put_str_in_pos(s, attr, pos / 80, pos % 80);
}

void syscall_init() {
    syscall_table[SYS_GET_PID] = sys_get_pid;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_READ] = sys_read;
    syscall_table[SYS_READ_CHAR] = sys_read_char;
    syscall_table[SYS_WRITE_IN_POS] = sys_write_in_pos;
}