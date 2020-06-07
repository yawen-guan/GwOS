#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "time.h"

#define IRQ0_FREQUENCY 100                               //时间中断的频率 100Hz
#define INPUT_FREQUENCY 1193180                          //计数器0的工作脉冲信号频率
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY  //计数器0的计数初值
#define CONTRER0_PORT 0x40                               //计数器0的端口
#define COUNTER0_NO 0                                    //选择计数器的号码
#define COUNTER_MODE 2                                   //使用方式2：比率发生器
#define READ_WRITE_LATCH 3                               //读写方式
#define PIT_CONTROL_PORT 0x43                            //控制字寄存器的端口

uint32_t ticks;  //自中断开启以来，内核总共的tick数

/**
 * @brief 设置时间中断的频率
 * 把操作的计数器counter_no、读写锁属性rwl、计数器模式counter_mode写入模式控制寄存器并赋予初始值counter_value
 */
void frequency_set(uint8_t counter_port,
                   uint8_t counter_no,
                   uint8_t rwl,
                   uint8_t counter_mode,
                   uint16_t counter_value) {
    // 写入控制字
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    // 写入counter_value的低8位
    outb(counter_port, (uint8_t)counter_value);
    // 写入counter_value的高8位
    outb(counter_port, (uint8_t)counter_value >> 8);
}

/**
 * @brief 时间中断处理函数
 * 
 */
void intr_timer_handler() {
    struct pcb* now_thread = running_thread();

    now_thread->elapsed_ticks++;
    ticks++;
    if (now_thread->ticks == 0) {  // time slice用完
        schedule();
    } else
        now_thread->ticks--;
}

/**
 * @brief 计数器PIC8253的初始化
 * 
 */
void timer_init() {
    put_str("timer_init start\n", 0x07);
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("timer_init done\n", 0x07);
}
