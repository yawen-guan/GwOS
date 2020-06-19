#include "common.h"
#include "keyboard.h"
#include "print.h"
#include "stdint.h"
#include "stdio.h"
#include "syscall.h"

void prog5() {
    printf("Test: scanf char, string, int(10), int(16)\n");
    char c;
    char s[100];
    uint32_t x;
    uint32_t y;
    scanf("%c%s%d%x", &c, s, &x, &y);

    printf("\nTest: printf\n");
    printf("char = %c  string = %s  int(10) = %d  int(16) = %x\n", c, s, x, y);

    printf("\nTest: gets puts\n");
    printf("gets: ");
    gets(s);
    printf("\nputs: ");
    puts(s);
    printf("\n");

    printf("\nTest: getchar putchar\n");
    printf("getchar: ");
    c = getchar();
    printf("\nputchar: ");
    putchar(c);
    printf("\n");

    while (1) {
        call_2a();
    }
}