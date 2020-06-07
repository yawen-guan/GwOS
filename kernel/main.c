#include "print.h"
#include "init.h"
#include "string.h"
#include "memory.h"
#include "interrupt.h"

int main(void) {
    put_str("I am kernel\n", 0x07);
    init_all();
    intr_enable();    
    // put_char_pos('A', 0x07, 1 * 80 + 0);
    // put_char_in_pos('B', 0x07, 2, 0);
    // put_char_in_pos('\\', 0x07, 24, 79);
    // init_all();
    // put_str("\n get kernel page start vaddr is \n", 0x07);
    // void *addr = get_kernel_pages(3);
    // put_uint((uint32_t)addr, 16, 0x07);
    // put_char('\n', 0x07);
    // asm volatile("sti"); //开中断
    while(1);    
    return 0;
}