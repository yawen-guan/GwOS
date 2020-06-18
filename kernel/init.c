#include "init.h"

#include "console.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "syscall-init.h"
#include "thread.h"
#include "timer.h"
#include "tss.h"

void init_all() {
    // put_str("\ninit all\n", 0x07);
    interrupt_init();
    mem_init();
    thread_init();
    timer_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
}