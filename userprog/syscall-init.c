#include "syscall-init.h"

#include "console.h"
#include "exit.h"
#include "fork.h"
#include "global.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "syscall.h"
#include "thread.h"
#include "wait.h"
#include "sync.h"
#include "timer.h"

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

void sys_call_intr2a() {
    asm volatile("int $0x2a");
}

void syscall_init() {
    syscall_table[SYS_GET_PID] = sys_get_pid;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_READ] = sys_read;
    syscall_table[SYS_READ_CHAR] = sys_read_char;
    syscall_table[SYS_WRITE_IN_POS] = sys_write_in_pos;
    syscall_table[SYS_CALL_2A] = sys_call_intr2a;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE] = sys_free;
    syscall_table[SYS_FORK] = sys_fork;
    syscall_table[SYS_WAIT] = sys_wait;
    syscall_table[SYS_EXIT] = sys_exit;
    syscall_table[SYS_SLEEP] = ms_sleep;
    syscall_table[SYS_WAIT_WITHOUT_PID] = sys_wait_without_pid;
    syscall_table[SYS_ACQUIRE_LOCK] = lock_acquire;
    syscall_table[SYS_RELEASE_LOCK] = lock_release;
    syscall_table[SYS_INITIAL_LOCK] = lock_init;
}