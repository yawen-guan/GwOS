#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "string.h"
#include "thread.h"

#define SCREEN_SIZE 2000
#define COMMAND_SIZE 20

// void k_thread_a(void* arg);  //一定要先声明后调用！不然虚拟地址会出错
// void k_thread_b(void* arg);

// void u_prog_a(void);
// void u_prog_b(void);
// int test_var_a = 0, test_var_b = 0;

void clear();
void saves_creen();
void recover_screen();

extern void prog1();
extern void prog2();
extern void prog3();
extern void prog4();

uint32_t screen_buf[SCREEN_SIZE];
uint16_t cursor_pos;
// bool inputing;
// char command[COMMAND_SIZE];
// uint32_t command_len;

int main(void) {
    // put_str("I am kernel\n", 0x07);

    init_all();
    // put_str("here\n", 0x07);

    // thread_start("idle", 5, idle, NULL);
    // thread_start("terminal", 30, i)

    // console_acquire();
    clear();
    cursor_pos = 0;

    // // inputing = false;
    // // while (1) {
    // console_put_str("\n\rGwShell: ", 0x0E);

    // for(int i = 0; i < len; i ++){

    // }
    // // char c = keyboard_getchar();
    // thread_start("readin", 20, readin, NULL);

    // while (1)
    // ;
    // console_acquire();
    // inputing = true;

    // console_release();
    // int len = 0;
    // while (inputing == true && len < COMMAND_SIZE) {
    //     //     // len++;
    //     command[len++] = keyboard_getchar();
    // }
    // command[len] = 0;
    // inputing = false;
    // console_release();
    // debug_printf_s("command = ", command);

    // if (strcmp(command, "help") == 0) {
    // } else if (strcmp(command, "ls") == 0) {
    // } else if (strcmp(command, "exec") == 0) {
    // } else if (strcmp(command, "clear") == 0) {
    // }
    // }

    // console_release();

    // while (1) {

    // }

    // put_char_in_pos('c', 0x07, 0, 0);
    // uint32_t c = get_char_pos(0);
    // debug_printf_uint("c = ", 'c');
    // debug_printf_uint("c = ", (c & 0xFF00) >> 8);
    // debug_printf_uint("c = ", (c & 0x00FF));
    // put_char((c & 0xFF00) >> 8, c & 0xFF);

    thread_start("prog1", 30, prog1, NULL);
    thread_start("prog2", 30, prog2, NULL);
    thread_start("prog3", 30, prog3, NULL);
    thread_start("prog4", 30, prog4, NULL);
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");

    // inputing = true;
    // thread_start("k_thread_a", 31, k_thread_a, "argA ");

    // console_put_str(command, 0x07);

    // thread_start("k_thread_b", 8, k_thread_b, "argB ");
    intr_enable();

    while (1) {
    };
    return 0;
}

void clear() {
    set_cursor_in_pos(0, 0);
    for (int i = 0; i < 2000; i++) console_put_char(0, 0x07);
    set_cursor_in_pos(0, 0);
}

void save_screen() {
    cursor_pos = get_cursor_pos();
    for (int i = 0; i < SCREEN_SIZE; i++) {
        screen_buf[i] = get_char_pos(i);
    }
}

void recover_screen() {
    set_cursor_in_pos(0, 0);
    for (int i = 0; i < SCREEN_SIZE; i++) {
        put_char((screen_buf[i] & 0xFF00) >> 8, (screen_buf[i] & 0xFF));
    }
    set_cursor(cursor_pos);
}

void idle() {
}

// void readin() {
//     // inputing = true;
//     char c = keyboard_getchar();
// }

// void k_thread_a(void* arg) {
//     char* s = arg;
//     while (inputing == true) {
//         enum intr_status old_status = intr_disable();
//         command_len = 0;
//         while (!is_ioq_empty(&keyboard_ioq) && command_len < COMMAND_SIZE) {
//             char c = ioq_getchar(&keyboard_ioq);
//             if (c == '\r') {
//                 inputing = false;
//                 break;
//             }
//             command[command_len++] = c;
//             console_put_str(s, 0x07);
//             console_put_char(c, 0x07);
//         }
//         command[command_len] = 0;
//         intr_set_status(old_status);
//     }
// }

// /* 在线程中运行的函数 */
// void k_thread_b(void* arg) {
//     /* 用void*来通用表示参数,被调用的函数知道自己需要什么类型的参数,自己转换再用 */
//     char* s = arg;
//     while (1) {
//         intr_disable();
//         enum intr_status old_status = intr_disable();
//         console_put_str(s, 0x07);
//         // char c = ioq_getchar(&keyboard_ioq);
//         intr_set_status(old_status);
//         intr_enable();
//     }
// }

// /* 测试用户进程 */
// void u_prog_a(void) {
//     while (1) {
//         test_var_a++;
//     }
// }

// /* 测试用户进程 */
// void u_prog_b(void) {
//     while (1) {
//         test_var_b++;
//     }
// }
