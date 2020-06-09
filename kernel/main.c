#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "string.h"
#include "sync.h"
#include "thread.h"

#define SCREEN_SIZE 2000
#define COMMAND_SIZE 50

// void k_thread_a(void* arg);  //一定要先声明后调用！不然虚拟地址会出错
// void k_thread_b(void* arg);

// void u_prog_a(void);
// void u_prog_b(void);
// int test_var_a = 0, test_var_b = 0;
void terminal(void *arg);
void userprog_init();
void run_prog1(void *arg);
void run_prog2(void *arg);
void run_prog3(void *arg);
void run_prog4(void *arg);

void saves_creen();
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

// bool inputing;
char command[COMMAND_SIZE];

static struct lock prog1_lock;
static struct lock prog2_lock;
static struct lock prog3_lock;
static struct lock prog4_lock;

static struct lock prog_lock[5];
// uint32_t command_len;

int main(void) {
    put_str("I am kernel\n", 0x07);

    init_all();

    userprog_init();
    thread_start("terminal", 30, terminal, NULL);

    intr_enable();

    while (1) {
    };
    return 0;
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
    console_clear();
    while (1) {
        console_acquire();
        put_str("\n\rGwShell:", 0x0E);
        getline();
        console_release();

        if (strcmp(command, "help") == 0) {
            console_put_str(msg_help, 0x07);
        } else if (strcmp(command, "clr") == 0) {
            console_clear();
        } else if (strcmp(command, "ls") == 0) {
            console_put_str(msg_ls, 0x07);
        } else if (strcmp(command, "exit") == 0) {
            console_clear();
        } else if (strcmp(command, "exec") == 0) {
            console_put_str("Please enter the program ID seperated by a space (e.g.  \"1 2 3 4\") : ", 0x0F);

            console_acquire();
            getline();

            // lock_release(&prog_lock[1]);
            // lock_release(&prog_lock[2]);
            for (int i = 0; i < strlen(command); i++) {
                if (command[i] >= '1' && command[i] <= '4') {
                    lock_release(&prog_lock[command[i] - '0']);
                } else if (command[i] != ' ') {
                    put_str("\nSorry, no such program ID.\n", 0x07);
                }
            }
            console_release();

            for (int i = 0; i < 100000; i++)
                for (int j = 0; j < 1000; j++) {}
        }
    }
}

void userprog_init() {
    // lock_init(&prog1_lock);
    // lock_acquire(&prog1_lock);

    for (int i = 1; i <= 4; i++) {
        lock_init(&prog_lock[i]);
        lock_acquire(&prog_lock[i]);
    }
    thread_start("run_prog1", 30, run_prog1, NULL);
    thread_start("run_prog2", 30, run_prog2, NULL);
    thread_start("run_prog3", 30, run_prog3, NULL);
    thread_start("run_prog4", 30, run_prog4, NULL);
}

void run_prog1(void *arg) {
    lock_acquire(&prog_lock[1]);
    prog1();
    lock_release(&prog_lock[1]);
}

void run_prog2(void *arg) {
    lock_acquire(&prog_lock[2]);
    prog2();
    lock_release(&prog_lock[2]);
}

void run_prog3(void *arg) {
    lock_acquire(&prog_lock[3]);
    prog3();
    lock_release(&prog_lock[3]);
}

void run_prog4(void *arg) {
    lock_acquire(&prog_lock[4]);
    prog4();
    lock_release(&prog_lock[4]);
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