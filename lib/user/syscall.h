#pragma once

#include "stdint.h"
#include "sync.h"

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
    SYS_WAIT_WITHOUT_PID,
    SYS_ACQUIRE_LOCK,
    SYS_RELEASE_LOCK,
    SYS_INITIAL_LOCK
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

void acquire_lock(struct lock *lock);

void release_lock(struct lock *lock);

void initial_lock(struct lock *lock);