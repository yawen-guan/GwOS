#pragma once

#include "stdint.h"
#include "sync.h"
#include "filesystem.h"

enum SYSCALL_FUNC {
    SYS_GET_PID,
    SYS_WRITE,
    SYS_READ,
    SYS_READ_CHAR,
    SYS_WRITE_IN_POS,
    SYS_CALL_2A,
    SYS_MALLOC,
    SYS_FREE,
    SYS_FORK,
    SYS_WAIT,
    SYS_EXIT,
    SYS_SLEEP,
    SYS_WAIT_WITHOUT_PID,
    SYS_ACQUIRE_LOCK,
    SYS_RELEASE_LOCK,
    SYS_INITIAL_LOCK,
    SYS_READ_DISK,
    SYS_WRITE_DISK,
    SYS_EXECV
};

uint32_t get_pid();

uint32_t write(char *s, uint32_t attr);

void read(char *s);

char read_char();

void write_in_pos(char *s, uint32_t attr, uint32_t pos);

void call_2a();

void *malloc(uint32_t size);

void free(void *ptr);

int16_t fork();

// int16_t wait(int32_t *child_exit_status);

int16_t wait(int32_t pid, int32_t *child_exit_status);

int16_t wait_without_pid(int32_t *child_exit_status);

void exit(int32_t status);

void sleep(uint32_t ms);

void acquire_lock(struct lock *lock);

void release_lock(struct lock *lock);

void initial_lock(struct lock *lock);

void read_disk(uint8_t *buf, int secID, bool is_sdb);

void write_disk(const uint8_t *buf, int secID, bool is_sdb);

int32_t execv(struct ActiveFile *acfile, const char *argv[]);