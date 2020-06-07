#include "bitmap.h"
#include "interrupt.h"
#include "string.h"
#include "print.h"

void bitmap_init(struct bitmap *mp){
    put_str("\nbitmap_init start\n", 0x07);
    put_str("\nmp -> len = ", 0x07);
    put_int(mp -> len, 10, 0x07);
    put_char('\n', 0x07);

    memset(mp -> bits, 0, mp -> len);
}

/** 
 * 判断bitmap的第idx位是否为1
 */ 
bool bitmap_check_idx(struct bitmap* mp, uint32_t idx){
    uint32_t idx_byte = idx / 8;
    uint32_t idx_bit = idx % 8;

    return (((mp -> bits[idx_byte]) & (1 << idx_bit)) != false);
}

/** 
 * 在bitmap中申请连续的cnt个位
 * @return 成功则返回起始位的下标，否则返回-1
 */
int32_t bitmap_apply_cnt(struct bitmap *mp, uint32_t cnt){
    for(int i = 0; i < mp -> len - cnt; i ++){
        if(bitmap_check_idx(mp, i) == true) continue;

        bool flag = true;
        for(int j = 1; j < cnt; j ++){
            if(bitmap_check_idx(mp, i + j) == true){
                flag = false;
                break;
            }
        }
        if(flag == true) return i;
    }
    return -1;
}

void bitmap_set_idx(struct bitmap *mp, uint32_t idx, uint8_t value){
    uint32_t idx_byte = idx / 8;
    uint32_t idx_bit = idx % 8;
    if(value == 0){
        mp -> bits[idx_byte] &= ~(1 << idx_bit);
    }
    else {
        mp -> bits[idx_byte] |= (1 << idx_bit);
    }
}
