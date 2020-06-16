#include "common.h"

void node_init(struct running_node* obj, char c, int32_t pos_x, int32_t pos_y, int32_t v_x, int32_t v_y) {
    obj->c = c;
    obj->pos_x = pos_x;
    obj->pos_y = pos_y;
    obj->v_x = v_x;
    obj->v_y = v_y;
}

void node_update(struct running_node* obj, uint32_t min_x, uint32_t min_y, uint32_t max_x, uint32_t max_y) {
    obj->pos_x += obj->v_x;
    obj->pos_y += obj->v_y;
    if (obj->pos_x == min_x) {
        obj->v_x = 1;
    }
    if (obj->pos_x == max_x - 1) {
        obj->v_x = -1;
    }
    if (obj->pos_y == min_y) {
        obj->v_y = 1;
    }
    if (obj->pos_y == max_y - 1) {
        obj->v_y = -1;
    }
}

void int_call() {
    asm volatile("int $0x2a");
}