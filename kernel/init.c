#include "init.h"

#include "console.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "timer.h"

void init_all() {
    put_str("\ninit all\n", 0x07);
    interrupt_init();
    mem_init();
    thread_init();
    timer_init();
    console_init();
    // keyboard_init();
}