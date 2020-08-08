#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"
#include "thread.h"
#include "ide.h"

#define SCREEN_SIZE 2000
#define COMMAND_SIZE 50

// void save_screen();
// void recover_screen();

extern void shell();

// uint32_t screen_buf[SCREEN_SIZE];
// uint16_t cursor_pos;

bool exec_flag;
bool release_flag;
uint32_t release_cnt;

int main() {
    init_all();
    intr_enable();
    thread_exit(running_thread(), true);
    return 0;
}

void init(void) {
    uint32_t ret_pid = fork();
    if (ret_pid) {  // father
        int32_t status;
        while (1) {
            int32_t child_pid = wait_without_pid(&status);
            if (child_pid == -1) continue;
            printf("I am father, my pid is %d, child pid is %d, child status = %d\n", get_pid(), child_pid, status);
        }

    } else {  // child
        shell();
    }
}


// void save_screen() {
//     cursor_pos = get_cursor_pos();
//     for (int i = 0; i < SCREEN_SIZE; i++) {
//         screen_buf[i] = get_char_pos(i);
//     }
// }

// void recover_screen() {
//     set_cursor_in_pos(0, 0);
//     for (int i = 0; i < SCREEN_SIZE; i++) {
//         put_char((screen_buf[i] & 0xFF00) >> 8, (screen_buf[i] & 0xFF));
//     }
//     set_cursor(cursor_pos);
// }
