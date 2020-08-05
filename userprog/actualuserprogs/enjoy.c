#include "stdio.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"

char fruit_disk = 0;  // 果盘
char words[15][15] = {0};
struct lock fruitlock;
struct lock wordslock;

void putwords(char *s) {
    acquire_lock(&wordslock);
    int cnt = 0, idx = 0;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == ' ' || s[i] == '!') {
            words[cnt][idx] = 0;
            cnt++;
            idx = 0;
            continue;
        }
        words[cnt][idx++] = s[i];
    }
    release_lock(&wordslock);
}
void putfruit() {
    acquire_lock(&fruitlock);
    fruit_disk = 1;
    release_lock(&fruitlock);
}

void myprintf() {
    for (int i = 0; i < 9; i++) {
        printf("%s ", words[i]);
        memset(words[i], 0, sizeof(words[i]));
    }
    printf("!\n");
}

void enjoy() {
    initial_lock(&fruitlock);
    initial_lock(&wordslock);
    int s = fork();
    if (s == 0) {
        while (1) {
            putwords("Father will live one year after anther for ever!");
        }
        exit(0);
    }
    int d = fork();
    if (d == 0) {
        while (1) {
            putfruit();
        }
        exit(0);
    }
    while (1) {
        acquire_lock(&fruitlock);
        acquire_lock(&wordslock);
        if (fruit_disk != 0 && sizeof(words[0]) != 0) {
            myprintf(words);
            fruit_disk = 0;
        }
        release_lock(&fruitlock);
        release_lock(&wordslock);
    }
}
