#pragma once

#include "stdint.h"
#include "sync.h"
#include "thread.h"

#define BUF_SIZE 64

//io circle queue
struct ioqueue {
    struct lock lock;
    struct pcb* producer;
    struct pcb* consumer;
    char buf[BUF_SIZE];
    int32_t head, tail;  // 实际为缓冲区的下标
};

/**
 * @brief 初始化io环状队列
 * 
 * @param q 
 */
void ioqueue_init(struct ioqueue* q);

/**
 * @brief 判断队列是否已满
 * 
 * @param q 
 * @return true 
 * @return false 
 */
bool is_ioq_full(struct ioqueue* q);
/**
 * @brief 判断队列是否为空
 * 
 * @param q 
 * @return true 
 * @return false 
 */
bool is_ioq_empty(struct ioqueue* q);

/**
 * @brief 从ioq队列中获取一个字符
 * 
 * @param q 
 * @return char 
 */
char ioq_getchar(struct ioqueue* q);

/**
 * @brief 向ioq队列中写入一个字符
 * 
 * @param q ioq队列
 * @param c 要写入的字符
 */
void ioq_putchar(struct ioqueue* q, char c);