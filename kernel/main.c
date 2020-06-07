#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

void k_thread_a(void *arg);  //一定要先声明后调用！不然虚拟地址会出错

int main(void) {
    put_str("I am kernel\n", 0x07);
    init_all();
    put_str("here\n", 0x07);

    thread_start("k_thread_a", 31, k_thread_a, "argA ");

    while (1)
        ;
    return 0;
}

void k_thread_a(void *arg) {
    char *s = arg;
    while (1) {
        put_str(s, 0x07);
    }
}
