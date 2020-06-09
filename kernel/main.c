#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "string.h"
#include "thread.h"

// void k_thread_a(void* arg);  //一定要先声明后调用！不然虚拟地址会出错
// void k_thread_b(void* arg);

// void u_prog_a(void);
// void u_prog_b(void);
// int test_var_a = 0, test_var_b = 0;

extern void prog1();
extern void prog2();
extern void prog3();
extern void prog4();

#define SCREEN_SIZE 2000

uint32_t screen_buf[SCREEN_SIZE];
uint16_t cursor_pos;

void clear();
void saves_creen();
void recover_screen();

int main(void) {
    put_str("I am kernel\n", 0x07);
    init_all();
    put_str("here\n", 0x07);
    clear();
    cursor_pos = 0;

    // console_acquire();
    // cursor_pos = get_cursor_pos();

    while (1) {
        console_put_str("\n\rGwShell: ", 0x0E);
    }

    console_release();

    // while (1) {

    // }

    // put_char_in_pos('c', 0x07, 0, 0);
    // uint32_t c = get_char_pos(0);
    // debug_printf_uint("c = ", 'c');
    // debug_printf_uint("c = ", (c & 0xFF00) >> 8);
    // debug_printf_uint("c = ", (c & 0x00FF));
    // put_char((c & 0xFF00) >> 8, c & 0xFF);

    // thread_start("prog1", 30, prog1, NULL);
    // thread_start("prog2", 30, prog2, NULL);
    // thread_start("prog3", 30, prog3, NULL);
    // thread_start("prog4", 30, prog4, NULL);
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");

    // thread_start("k_thread_a", 31, k_thread_a, "argA ");
    // thread_start("k_thread_b", 8, k_thread_b, "argB ");
    intr_enable();

    while (1) {
        // console_put_str("Main ", 0x07);
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

// void k_thread_a(void* arg) {
//     char* s = arg;
//     while (1) {
//         enum intr_status old_status = intr_disable();
//         console_put_str(s, 0x07);
//         // char c = ioq_getchar(&keyboard_ioq);
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
