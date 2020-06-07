#ifndef __LIB_KERNEL_IO_H
#define __LIB_KERNEL_IO_H

#include "../stdint.h"

//asm [volatile] ("assembly code" : output : input : clobber/modify)

/** 
 * 向端口port写入一个字节data
 */
static inline void outb(uint16_t port, uint8_t data){
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
    //a:用eax/ax, N:0~255, d:用edx/dx
    //b0:al  w1:dx
}

/**
 * 向端口port写入addr起始的cnt个word
 */
static inline void outsw(uint16_t port, const void *addr, uint32_t cnt){
    asm volatile("cld; rep outsw" : "+S"(addr), "+c"(cnt) : "d"(port));
    //+:作为输入和输出 S:esi/si c:ecx/cx
}

/**
 * 从端口port读取一个字节并返回
 */

static inline uint8_t inb(uint16_t port){
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
    //=:只作为输出
    return data;
}

/**
 * 从端口port读入cnt个word并写入addr
 */
static inline void insw(uint16_t port, void *addr, uint32_t cnt){
    asm volatile("cld; rep insw" : "+D"(addr), "+c"(cnt) : "d"(port));
}

#endif