#include "syscall.h"

//宏：最后一个语句为代码块的返回值

#define _syscall0(NUMBER) ({    \
    int retval;                 \
    asm volatile("int $0x80"    \
                 : "=a"(retval) \
                 : "a"(NUMBER)  \
                 : "memory");   \
    retval;                     \
})

#define _syscall1(NUMBER, ARG1) ({        \
    int retval;                           \
    asm volatile("int $0x80"              \
                 : "=a"(retval)           \
                 : "a"(NUMBER), "b"(ARG1) \
                 : "memory");             \
    retval;                               \
})

#define _syscall2(NUMBER, ARG1, ARG2) ({             \
    int retval;                                      \
    asm volatile("int $0x80"                         \
                 : "=a"(retval)                      \
                 : "a"(NUMBER), "b"(ARG1), "c"(ARG2) \
                 : "memory");                        \
    retval;                                          \
})

#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({                  \
    int retval;                                                 \
    asm volatile("int $0x80"                                    \
                 : "=a"(retval)                                 \
                 : "a"(NUMBER), "b"(ARG1), "c"(ARG2), "d"(ARG3) \
                 : "memory");                                   \
    retval;                                                     \
})

uint32_t get_pid() {
    return _syscall0(SYS_GET_PID);
}

uint32_t write(char *s, uint32_t attr) {
    return _syscall2(SYS_WRITE, s, attr);
}

void read(char *s) {
    _syscall1(SYS_READ, s);
}

char read_char() {
    return _syscall0(SYS_READ_CHAR);
}

void write_in_pos(char *s, uint32_t attr, uint32_t pos) {
    _syscall3(SYS_WRITE_IN_POS, s, attr, pos);
}

void call_2a() {
    _syscall0(SYS_CALL_2A);
}

void *malloc(uint32_t size) {
    return (void *)_syscall1(SYS_MALLOC, size);
}

void free(void *ptr) {
    _syscall1(SYS_FREE, ptr);
}

int16_t fork(){
    return _syscall0(SYS_FORK);
}

// int16_t wait(int32_t *child_exit_status){
    // return _syscall1(SYS_WAIT, child_exit_status);
// }

int16_t wait(int32_t pid, int32_t *child_exit_status){
    return _syscall2(SYS_WAIT, pid, child_exit_status);
}

int16_t wait_without_pid(int32_t *child_exit_status){
    return _syscall1(SYS_WAIT, child_exit_status);
}

void exit(int32_t status){
    _syscall1(SYS_EXIT, status);
}

void acquire_lock(struct lock *lock){
    _syscall1(SYS_ACQUIRE_LOCK, lock);
}

void release_lock(struct lock *lock){
    _syscall1(SYS_RELEASE_LOCK, lock);
}

void initial_lock(struct lock *lock){
    _syscall1(SYS_INITIAL_LOCK, lock);
}