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

void terminal(void* arg);
static void intr_handler_0x2a();
static void intr_handler_0x2b();

void save_screen();
void recover_screen();

extern void prog1();
extern void prog2();
extern void prog3();
extern void prog4();
extern void prog5();
void progtest();

uint32_t screen_buf[SCREEN_SIZE];
uint16_t cursor_pos;
char* msg_help =
    "Welcome to GwShell. Internal commands are as following.\n\r"
    "help               show help information.\n\r"
    "ls                 list the information of user programs.\n\r"
    "clr                clear the screen.\n\r"
    "exec               run the several user program at the same time.\n\r"
    "intr               call interrupt\n\r"
    "                   Not limit to one program.\n\r"
    "exit               exit the current GwShell.\n\r";
char* msg_ls =
    "ID      FILENAME       TYPE\n\r"
    "1       prog1          com\n\r"
    "2       prog2          com\n\r"
    "3       prog3          com\n\r"
    "4       prog4          com\n\r"
    "5       prog5          com\n\r";

extern struct ioqueue keyboard_ioq;

bool exec_flag;
bool release_flag;
uint32_t release_cnt;

// void idle();
// void k_thread_a(void*);
// void k_thread_b(void*);
// void u_prog_a(void);
// void u_prog_b(void);
// int test_var_a = 0, test_var_b = 0;

void k_thread_a(void* arg);
void k_thread_b(void* arg);
void u_prog_a(void);
void u_prog_b(void);

void init();

int main(void) {
    init_all();
    
    printf("I am kernel\n");
    // process_execute(u_prog_a, "u_prog_a");
    // process_execute(init, "init");
    printf("I am kernel\n");
    intr_enable();
    /********  测试代码  ********/
    /********  测试代码  ********/
    while (1)
        ;
    return 0;
}

void u_prog_a() {
    printf("u_prog_a\n");
    while (1)
        ;
}

void init(void) {
    printf("init\n");
    uint32_t ret_pid = fork();
    if (ret_pid) {
        printf("i am father, my pid is %d, child pid is %d\n", get_pid(), ret_pid);
    } else {
        printf("i am child, my pid is %d, ret pid is %d\n", get_pid(), ret_pid);
    }
    while (1)
        ;
}

// int main(void) {
//     init_all();
//     register_handler(0x2A, intr_handler_0x2a);
//     register_handler(0x2B, intr_handler_0x2b);
//     thread_start("terminal", 30, terminal, NULL);
//     intr_enable();

//     while (1) {
//     };
//     return 0;
// }

// void idle() {
//     while (1)
//         ;
// }

void getline(char* command) {
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

void terminal(void* arg) {
    char command[COMMAND_SIZE];
    console_acquire();
    clear();
    while (1) {
        put_str("\n\rGwShell:", 0x0E);
        getline(command);

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

            getline(command);

            bool wrongID = false;
            for (int i = 0; i < strlen(command); i++) {
                if (command[i] == ' ') continue;
                if (command[i] >= '1' && command[i] <= '5') continue;
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
                if (command[i] >= '1' && command[i] <= '5') {
                    release_cnt++;
                    int id = command[i] - '0';
                    if (id == 1) process_execute(prog1, "prog1");
                    if (id == 2) process_execute(prog2, "prog2");
                    if (id == 3) process_execute(prog3, "prog3");
                    if (id == 4) process_execute(prog4, "prog4");
                    if (id == 5) process_execute(prog5, "prog5");
                }
            }
            console_release();
            while (exec_flag == true)
                ;

            console_acquire();
            clear();
            recover_screen();
        } else if (strcmp(command, "intr") == 0) {
            put_str("Available interrupt code: \n      0x2a(not available in terminal), 0x2b, 0x80(system call)\n\n", 0x0F);
            put_str("Please enter the interrupt code : ", 0x0F);
            getline(command);
            if (strcmp(command, "0x2b") == 0) {
                asm volatile("int $0x2b");
            } else if (strcmp(command, "0x80") == 0) {
                printf("Available syscall code: \n");
                printf("      0: get current PID\n");
                printf("      1: read and write a string\n");
                printf("      2: read and write a string in particular position\n");
                printf("Please enter the syscall code: ");
                scanf("%s", command);
                if (strcmp(command, "0") == 0) {
                    printf("The terminal's PID = %d\n", get_pid());
                } else if (strcmp(command, "1") == 0) {
                    printf("Please input a string to be written: ");
                    getline(command);
                    printf("%s\n", command);
                } else if (strcmp(command, "2") == 0) {
                    printf("Please input a string to be written, row, column: ");
                    uint32_t pos_x, pos_y;
                    scanf("%s%d%d", command, &pos_x, &pos_y);
                    write_in_pos(command, 0x07, pos_x * 80 + pos_y);
                } else {
                    printf("Not available syscall code.\n");
                }
            } else {
                put_str("Not available interrupt code.\n", 0x0F);
            }
        }
    }
    console_release();
}

static void intr_handler_0x2a() {
    if (release_flag == false) return;

    struct pcb* now = running_thread();

    if (now->status == TASK_RUNNING) release_cnt--;
    now->status = TASK_DIED;

    if (release_cnt == 0) exec_flag = false;
}

static void intr_handler_0x2b() {
    console_put_str_in_pos("int 2b", 0x0B, 24, 36);
    for (int i = 0; i < 50000000; i++)
        ;
    console_put_str_in_pos("      ", 0x0B, 24, 36);
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
