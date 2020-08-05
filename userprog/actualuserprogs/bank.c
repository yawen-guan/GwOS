#include "stdio.h"
#include "syscall.h"
#include "sync.h"

int bankbalance = 1000; // 银行帐户余额1000元

struct lock banklock;

void bank() {
    int sem_id;
    int t, totalsave = 0, totaldraw = 0;
    initial_lock(&banklock);
    int pid = fork();
    if (pid == -1) {
        printf("Error in fork!");
        exit(-1);
    }
    else if (pid) { // father 父进程反复存钱，每次10元
        while (1) {
            acquire_lock(&banklock);
            t = bankbalance;
            t += 10;
            bankbalance = t;
            release_lock(&banklock);
            totalsave += 10;
            printf("bankbalance = %d, totalsave = %d\n", t, totalsave);
        }
        exit(0);
    }
    else { // child 子进程反复取钱，每次20元
        while(1){
            acquire_lock(&banklock);
            t = bankbalance;
            t -= 20;
            bankbalance = t;
            release_lock(&banklock);
            totaldraw += 20;
            printf("bankbalance = %d, totaldraw = %d\n", t, totaldraw);
        }
        exit(0);
    }
}