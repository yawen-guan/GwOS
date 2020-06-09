#pragma once

#include "stdint.h"

#define delay1 60000
#define delay2 20

struct running_node {
    int32_t pos_x, pos_y, v_x, v_y;
    char c;
};

void node_init(struct running_node* obj, char c, int32_t pos_x, int32_t pos_y, int32_t v_x, int32_t v_y);

void node_update(struct running_node* obj, uint32_t min_x, uint32_t min_y, uint32_t max_x, uint32_t max_y);