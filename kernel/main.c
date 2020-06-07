#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

void k_thread_a(void* arg);  //一定要先声明后调用！不然虚拟地址会出错
void k_thread_b(void* arg);

int main(void) {
    put_str("I am kernel\n", 0x07);
    init_all();
    put_str("here\n", 0x07);

    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 8, k_thread_b, "argB ");

    intr_enable();

    while (1) {
        put_str("Main ", 0x07);
    };
    return 0;
}

void k_thread_a(void* arg) {
    char* s = arg;
    while (1) {
        put_str(s, 0x07);
    }
}

/* 在线程中运行的函数 */
void k_thread_b(void* arg) {
    /* 用void*来通用表示参数,被调用的函数知道自己需要什么类型的参数,自己转换再用 */
    char* para = arg;
    while (1) {
        put_str(para, 0x07);
    }
}
