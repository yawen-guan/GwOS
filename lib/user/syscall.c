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