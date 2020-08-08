#include "syscall-init.h"

#include "console.h"
#include "exec.h"
#include "exit.h"
#include "fork.h"
#include "global.h"
#include "ide.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"
#include "thread.h"
#include "timer.h"
#include "wait.h"

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

void sys_read_disk(uint8_t* buf, int secID, bool is_sdb) {
    if (is_sdb)
        ide_read(sdb, secID, buf, 1);
    else {
        ide_read(sda, secID, buf, 1);
    }
}

void sys_write_disk(const uint8_t* buf, int secID, bool is_sdb) {
    if (is_sdb) {
        ide_write(sdb, secID, buf, 1);
        // if (secID == 32) {
        //     ide_read(sdb, secID, buf, 1);
        //     kprintf("in sys_write_disk: sdb 32\n");
        //     for (int i = 0; i < 512; i++)
        //         kprintf("%x ", buf[i]);
        //     kprintf("\n");
        // }
    }
    
    else
        ide_write(sda, secID, buf, 1);
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
    syscall_table[SYS_READ_DISK] = sys_read_disk;
    syscall_table[SYS_WRITE_DISK] = sys_write_disk;
    syscall_table[SYS_EXECV] = sys_execv;
}