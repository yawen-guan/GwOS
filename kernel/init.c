#include "init.h"

#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "timer.h"

void init_all() {
    put_str("\ninit all\n", 0x07);
    interrupt_init();
    timer_init();
    mem_init();
    // keyboard_init();
}