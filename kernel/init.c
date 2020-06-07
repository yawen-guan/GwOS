#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "memory.h"
#include "keyboard.h"

void init_all(){
    put_str("\ninit all\n", 0x07);
    interrupt_init();
    mem_init();
    keyboard_init();    
}