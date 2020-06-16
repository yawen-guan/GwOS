// #ifndef __KERNEL_GLOBAL_H
// #define __KERNEL_GLOBAL_H

#pragma once

#include "stdint.h"

// ----- GDT 描述符 -----
// boot.inc里有，现在转成c的。含义没有变化

struct gdt_desc {
    uint16_t limit_low_word;
    uint16_t base_low_word;
    uint8_t base_mid_byte;
    uint8_t attr_low_byte;
    uint8_t limit_high_attr_high;
    uint8_t base_high_byte;
};

#define DESC_G_4K 1
#define DESC_D_32 1
#define DESC_L 0
#define DESC_AVL 0
#define DESC_P 1
#define DESC_DPL_0 0
#define DESC_DPL_1 1
#define DESC_DPL_2 2
#define DESC_DPL_3 3

#define DESC_S_CODE 1
#define DESC_S_DATA DESC_S_CODE
#define DESC_S_SYS 0
#define DESC_TYPE_CODE 8
#define DESC_TYPE_DATA 2
#define DESC_TYPE_TSS 9

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_KERNEL_CODE ((1 << 3) + (TI_GDT << 2) + RPL0)  //指向内核代码段的selector
#define SELECTOR_KERNEL_DATA ((2 << 3) + (TI_GDT << 2) + RPL0)  //指向内核数据段的selector
#define SELECTOR_KERNEL_STACK SELECTOR_KERNEL_DATA              //指向内核栈段的selector
#define SELECTOR_KERNEL_GS ((3 << 3) + (TI_GDT << 2) + RPL0)    //指向内核显存段的selector

#define SELECTOR_USER_CODE ((5 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_USER_DATA ((6 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_USER_STACK SELECTOR_USER_DATA

#define GDT_ATTR_HIGH ((DESC_G_4K << 7) + (DESC_D_32 << 6) + (DESC_L << 5) + (DESC_AVL << 4))
#define GDT_CODE_ATTR_LOW_DPL3 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE)
#define GDT_DATA_ATTR_LOW_DPL3 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA)

//----- TSS描述符  -----
#define TSS_DESC_D 0

#define TSS_ATTR_HIGH ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW ((DESC_P << 7) + (DESC_DPL_0 << 5) + (DESC_S_SYS << 4) + DESC_TYPE_TSS)
#define SELECTOR_TSS ((4 << 3) + (TI_GDT << 2) + RPL0)

//----- 中断IDT描述符 -----
#define IDT_DESC_P 1
#define IDT_DESC_DPL0 0
#define IDT_DESC_DPL3 3
#define IDT_DESC_32_TYPE 0xE  // 32位的门
#define IDT_DESC_16_TYPE 0x6  // 16位的门
#define IDT_DESC_ATTR_DPL0 ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)

//----- eflag -----
#define EFLAGS_MBS (1 << 1)   // 此项必须要设置
#define EFLAGS_IF_1 (1 << 9)  // if为1,开中断
#define EFLAGS_IF_0 0         // if为0,关中断
//IOPL 输入输出特权级
#define EFLAGS_IOPL_3 (3 << 12)  // IOPL3(用于测试用户程序在非系统调用下进行IO)
#define EFLAGS_IOPL_0 (0 << 12)  // IOPL0

// ----- usage -----
#define bool int
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
#define true 1
#define false 0
#define NULL ((void *)0)

// ----- page -----
#define PG_SIZE 4096

extern bool exec_flag;
extern bool release_flag;
