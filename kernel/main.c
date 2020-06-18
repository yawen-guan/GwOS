#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"
#include "thread.h"

#define SCREEN_SIZE 2000
#define COMMAND_SIZE 50

void terminal(void *arg);
static void intr_handler_0x2a();
static void intr_handler_0x2b();

void save_screen();
void recover_screen();

extern void prog1();
extern void prog2();
extern void prog3();
extern void prog4();

uint32_t screen_buf[SCREEN_SIZE];
uint16_t cursor_pos;
char *msg_help =
    "Welcome to GwShell. Internal commands are as following.\n\r"
    "help               show help information.\n\r"
    "ls                 list the information of user programs.\n\r"
    "clr                clear the screen.\n\r"
    "exec               run the several user program at the same time.\n\r"
    "                   Not limit to one program.\n\r"
    "exit               exit the current GwShell.\n\r";
char *msg_ls =
    "ID      FILENAME       TYPE\n\r"
    "1       prog1          com\n\r"
    "2       prog2          com\n\r"
    "3       prog3          com\n\r"
    "4       prog4          com\n\r";

extern struct ioqueue keyboard_ioq;

char command[COMMAND_SIZE];
bool exec_flag;
bool release_flag;
uint32_t release_cnt;

void idle();
void k_thread_a(void *);
void k_thread_b(void *);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0, test_var_b = 0;

int main(void) {
    put_str("I am kernel\n", 0x07);

    init_all();

    register_handler(0x2A, intr_handler_0x2a);
    register_handler(0x2B, intr_handler_0x2b);
    thread_start("terminal", 30, terminal, NULL);

    // thread_start("k_thread_a", 31, k_thread_a, "argA ");
    // thread_start("k_thread_b", 31, k_thread_b, "argB ");
    // thread_start("idle", 30, idle, NULL);

    // console_debug_printf_uint("console pid = ", get_pid(), 10);
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");

    // printf("%x %d %s %c\n", 10, 11, "hello_world!", 'c');
    // putchar('t');
    // putchar('\n');

    // char tmp[100];
    // read(tmp);

    // console_debug_printf_str("tmp = ", tmp);

    // uint32_t x;
    // uint32_t y;
    // char ch;
    // scanf("%d %x %s %c", &x, &y, tmp, &ch);

    // console_debug_printf_uint("x = ", x, 10);
    // console_debug_printf_uint("y = ", y, 10);
    // console_debug_printf_str("tmp = ", tmp);
    // console_debug_printf_str("ch = ", &ch);

    // write("testing write in main thread\n", 0x07);

    // ch = getchar();
    // console_debug_printf_str("ch = ", &ch);

    console_debug_printf_str("here ", "ok");

    intr_enable();

    while (1) {
    };
    return 0;
}
void idle() {}

/* 在线程中运行的函数 */
void k_thread_a(void *arg) {
    char *para = arg;
    console_put_str("k_thread_a is running\n", 0x07);
    // console_debug_printf_uint("pid = ", get_pid(), 10);
    // write("testing write\n", 0x07);
    while (1) {
        console_put_str(" v_a:0x", 0x07);
        console_put_int(test_var_a, 16, 0x07);
    }
}

/* 在线程中运行的函数 */
void k_thread_b(void *arg) {
    char *para = arg;
    console_put_str("k_thread_b is running\n", 0x07);
    // console_debug_printf_uint("pid = ", get_pid(), 10);
    while (1) {
        console_put_str(" v_b:0x", 0x07);
        console_put_int(test_var_b, 16, 0x07);
    }
}

/* 测试用户进程 */
void u_prog_a(void) {
    // test_var_a = get_pid();
    while (1) {
        test_var_a++;
    }
}

/* 测试用户进程 */
void u_prog_b(void) {
    while (1) {
        test_var_b++;
    }
}

void getline() {
    enum intr_status old_status = intr_disable();
    int len = 0;
    while (1) {
        char c = ioq_getchar(&keyboard_ioq);
        if (c == '\b' && len > 0) {
            len--;
            continue;
        }
        if (c == '\r' || c == '\n' || len >= COMMAND_SIZE) break;
        command[len++] = c;
    }
    command[len] = 0;
    intr_set_status(old_status);
}

void terminal(void *arg) {
    console_acquire();
    clear();
    while (1) {
        put_str("\n\rGwShell:", 0x0E);
        getline();

        if (strcmp(command, "help") == 0) {
            put_str(msg_help, 0x07);
        } else if (strcmp(command, "clr") == 0) {
            clear();
        } else if (strcmp(command, "ls") == 0) {
            put_str(msg_ls, 0x07);
        } else if (strcmp(command, "exit") == 0) {
            clear();
        } else if (strcmp(command, "exec") == 0) {
            put_str("Please enter the program ID seperated by a space (e.g.  \"1 2 3 4\") : ", 0x0F);

            getline();

            bool wrongID = false;
            for (int i = 0; i < strlen(command); i++) {
                if (command[i] == ' ') continue;
                if (command[i] >= '1' && command[i] <= '4') continue;
                put_str("\nSorry, no such program ID.\n", 0x07);
                wrongID = true;
            }
            if (wrongID == true) continue;

            save_screen();
            clear();

            exec_flag = true;
            release_flag = false;
            release_cnt = 0;
            for (int i = 0; i < strlen(command); i++) {
                if (command[i] >= '1' && command[i] <= '4') {
                    release_cnt++;
                    int id = command[i] - '0';
                    if (id == 1) thread_start("prog1", 30, prog1, NULL);
                    if (id == 2) thread_start("prog2", 30, prog2, NULL);
                    if (id == 3) thread_start("prog3", 30, prog3, NULL);
                    if (id == 4) thread_start("prog4", 30, prog4, NULL);
                }
            }
            console_release();
            while (exec_flag == true)
                ;

            console_acquire();
            clear();
            recover_screen();
        }
    }
    console_release();
}

static void intr_handler_0x2a() {
    if (release_flag == false) return;

    struct pcb *now = running_thread();

    if (now->status == TASK_RUNNING) release_cnt--;
    now->status = TASK_DIED;

    if (release_cnt == 0) exec_flag = false;
}

static void intr_handler_0x2b() {
    console_put_str_in_pos("int 2b", 0x0B, 24, 36);
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