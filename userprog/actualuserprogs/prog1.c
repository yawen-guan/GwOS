#include "common.h"
#include "keyboard.h"
#include "print.h"
#include "stdint.h"
#include "syscall.h"

void prog1() {
    uint32_t min_x = 0, max_x = 12;
    uint32_t min_y = 0, max_y = 40;

    struct running_node node1;
    struct running_node node2;
    struct running_node node3;
    struct running_node node4;
    node_init(&node1, 'A', 6, 0, 1, 1);
    node_init(&node2, 'B', 6, 39, 1, -1);
    node_init(&node3, 'C', 0, 0, 1, 1);
    node_init(&node4, 'D', 0, 39, 1, -1);

    uint32_t color = 0;
    while (1) {
        call_2a();
        for (int i = 0; i < delay1; i++) {
            for (int j = 0; j < delay2; j++) {
            }
        }
        if (color == 0xFF)
            color = 0;
        else
            color++;
        node_update(&node1, min_x, min_y, max_x, max_y);
        node_update(&node2, min_x, min_y, max_x, max_y);
        node_update(&node3, min_x, min_y, max_x, max_y);
        node_update(&node4, min_x, min_y, max_x, max_y);
        write_in_pos(&node1.c, color, node1.pos_x * 80 + node1.pos_y);
        write_in_pos(&node2.c, color, node2.pos_x * 80 + node2.pos_y);
        write_in_pos(&node3.c, color, node3.pos_x * 80 + node3.pos_y);
        write_in_pos(&node4.c, color, node4.pos_x * 80 + node4.pos_y);
        // put_char_in_pos(node1.c, color, node1.pos_x, node1.pos_y);
        // put_char_in_pos(node2.c, color, node2.pos_x, node2.pos_y);
        // put_char_in_pos(node3.c, color, node3.pos_x, node3.pos_y);
        // put_char_in_pos(node4.c, color, node4.pos_x, node4.pos_y);
    }
}