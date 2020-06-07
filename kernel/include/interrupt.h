#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "stdint.h"

enum intr_status{
    INTR_OFF,
    INTR_ON
};

/**
 * @brief 中断初始化
*/
void interrupt_init();

/**
 * @brief 获取中断的状态
 */
enum intr_status intr_get_status();

/**
 * @brief 设置中断的状态
 * 
 * @param status 
 * @return enum intr_status 
 */
enum intr_status intr_set_status(enum intr_status status);

/** 
 * @brief 开中断
 * @return 开中断之前的状态
 */
enum intr_status intr_enable();

/** 
 * @brief 关中断
 * @return 关中断之前的状态
 */
enum intr_status intr_disable();


#endif
