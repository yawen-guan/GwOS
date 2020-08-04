#include "stdint.h"
#include "stdio.h"
#include "syscall.h"
#include "thread.h"

char chararray[] = {'a', 'b', 'c', 'd', 'a', 'b', 'c', '1', '2', '3', '4', '5'};

void multiproc() {
    printf("Begin multi-process program ... \n");
    uint32_t p1 = fork();
    if (p1 == 0) {  // child 1
        int sum = 0;
        for (int i = 0; i < sizeof(chararray); i++) {
            if ((chararray[i] >= 'a' && chararray[i] <= 'z') || (chararray[i] >= 'A' && chararray[i] <= 'Z')) sum++;
        }
        printf("I am child counting letters, my pid is %d, I found %d letters\n", get_pid(), sum);
        exit(sum);
        return;
    }

    int32_t status, child_pid, finalsum = 0;
    child_pid = wait(p1, &status);
    finalsum += status;

    uint32_t p2 = fork();
    if (p2 == 0) {  // child 2
        int sum = 0;
        for (int i = 0; i < sizeof(chararray); i++) {
            if (chararray[i] >= '0' && chararray[i] <= '9') sum++;
        }
        printf("I am child counting numbers, my pid is %d, I found %d numbers\n", get_pid(), sum);
        exit(sum);
        return;
    }

    child_pid = wait(p2, &status);
    finalsum += status;
    printf("Final sum = %d\n", finalsum);

    exit(0);
}