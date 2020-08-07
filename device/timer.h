#pragma once

#include "global.h"
#include "stdint.h"

/**
 * @brief 计数器PIC8253的初始化
 * 
 */
void timer_init();

void ms_sleep(uint32_t ms);
