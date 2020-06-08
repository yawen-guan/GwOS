#include "debug.h"

#include "interrupt.h"
#include "print.h"

/* 打印文件名,行号,函数名,条件并使程序悬停 */
void panic_spin(char* filename,
                int line,
                const char* func,
                const char* condition) {
    intr_disable();  // 因为有时候会单独调用panic_spin,所以在此处关中断。
    put_str("\n\n\n!!!!! error !!!!!\n", 0x07);
    put_str("filename:", 0x07);
    put_str(filename, 0x07);
    put_str("\n", 0x07);
    put_str("line:", 0x07);
    put_int(line, 10, 0x07);
    put_str("\n", 0x07);
    put_str("function:", 0x07);
    put_str((char*)func, 0x07);
    put_str("\n", 0x07);
    put_str("condition:", 0x07);
    put_str((char*)condition, 0x07);
    put_str("\n", 0x07);
    while (1, 0x07)
        ;
}
