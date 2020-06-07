#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H

#include "stdint.h"
#include "global.h"

struct bitmap {
    uint32_t len; //位图的字节长度
    uint8_t *bits;
};

void bitmap_init(struct bitmap *mp);

/** 
 * 判断bitmap的第idx位是否为1
 */ 
bool bitmap_check_idx(struct bitmap* mp, uint32_t idx);

/** 
 * 在bitmap中申请连续的cnt个位
 * @return 成功则返回起始位的下标，否则返回-1
 */
int32_t bitmap_apply_cnt(struct bitmap *mp, uint32_t cnt);

void bitmap_set_idx(struct bitmap *mp, uint32_t idx, uint8_t value);




#endif