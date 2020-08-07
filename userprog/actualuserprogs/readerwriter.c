#include "stdio.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"

// DataType The_Data;
char data = 'a';

int reader_counter = 0, writer_counter = 0;
struct lock rdr, wrt, m, x, y;

// Main() {
//     Create_thread(reader, 1);
//     …;
//     Create_thread(reader, N); /*创建n个读者线程；*/
//     Create_thread(writer, 1);
//     …;
//     Create_thread(writer, M); /*创建m个写者线程；*/
// }

void reader(int pid) {
    // printf("reader\n");
    acquire_lock(&rdr);
    release_lock(&rdr);
    acquire_lock(&x);
    reader_counter++;
    if (reader_counter == 1) {
        acquire_lock(&wrt);
        acquire_lock(&m);
    }
    release_lock(&x);
    // printf("reader\n");
    printf("data is %c from reader pid = %d\n", data, pid);
    // sleep(2000);
    acquire_lock(&x);
    reader_counter--;
    if (reader_counter == 0) {
        release_lock(&wrt);
        release_lock(&m);
    }
    release_lock(&x);
}
// reader(i) {
//     p(rdr);538436305 538430000
//     v(rdr);
//     p(x);
//     reader_counter++;
//     if (reader_counter == 1) {
//         p(wrt);
//         p(m);
//     }
//     v(x);
//     reading(The_Data);
//     p(x);
//     reader_counter--;
//     if (reader_counter == 0) {
//         v(wrt);
//         v(m);
//     }
//     v(x);
// }

void writer(int pid) {
    // printf("writer\n");
    acquire_lock(&wrt);
    release_lock(&wrt);
    acquire_lock(&y);
    writer_counter++;
    if (writer_counter == 1) {
        acquire_lock(&rdr);
    }
    release_lock(&y);
    acquire_lock(&m);
    data = 'a' + pid % 26;
    printf("data is %c from writer pid = %d\n", data, pid);
    // sleep(2000);
    release_lock(&m);
    acquire_lock(&y);
    writer_counter--;
    if (writer_counter == 0) {
        release_lock(&rdr);
    }
    release_lock(&y);
}

// writer(i) {
//     p(wtr);
//     v(wtr);
//     p(y);
//     writer_counter++;
//     if (writer_counter == 1) p(rdr);
//     v(y);
//     p(m);
// }
// writing(The_Data);
// v(m);
// }
// p(y);
// writer_counter--;
// if (writer_counter == 0) v(rdr);
// v(y);
// }
// }

void readerwriter() {
    initial_lock(&rdr);
    initial_lock(&wrt);
    initial_lock(&m);
    initial_lock(&x);
    initial_lock(&y);
    //3 writers
    int p1 = fork();
    if (p1 == 0) {
        int pid = get_pid();
        while (1) writer(pid);
        exit(0);
    }
    int p2 = fork();
    if (p2 == 0) {
        int pid = get_pid();
        while (1) writer(pid);
        exit(0);
    }
    int p3 = fork();
    if (p3 == 0) {
        int pid = get_pid();
        while (1) writer(pid);
        exit(0);
    }
    // 3 readers
    int p4 = fork();
    if (p4 == 0) {
        int pid = get_pid();
        while (1) reader(pid);
        exit(0);
    }
    int p5 = fork();
    if (p5 == 0) {
        int pid = get_pid();
        while (1) reader(pid);
        exit(0);
    }
    int p6 = fork();
    if (p6 == 0) {
        int pid = get_pid();
        while (1) reader(pid);
        exit(0);
    }

    
    while (1)
        ;
    exit(0);
}
