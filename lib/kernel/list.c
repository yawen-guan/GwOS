#include "list.h"

#include "interrupt.h"

/**
 * @brief 初始化环形双向列表
 * 
 */
void list_init(struct list* list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

/**
 * @brief 将节点node插入另一个节点src之前
 * 
 */
void list_insert_before(struct list_node* src, struct list_node* node) {
    enum intr_status old_status = intr_disable();

    src->prev->next = node;
    node->prev = src->prev;
    node->next = src;
    src->prev = node;

    intr_set_status(old_status);
}

/**
 * @brief 让node成为队头head的下一个节点
 * 
 */
void list_push(struct list* list, struct list_node* node) {
    list_insert_before(list->head.next, node);
}

/**
 * @brief 在队尾tail前插入node
 * 
 */
void list_append(struct list* list, struct list_node* node) {
    list_insert_before(&list->tail, node);  // 在队尾的前面插入
}

/**
 * @brief 删除node
 * 
 */
void list_remove(struct list_node* node) {
    enum intr_status old_status = intr_disable();

    node->prev->next = node->next;
    node->next->prev = node->prev;

    intr_set_status(old_status);
}

/**
 * @brief 弹出head.next, 也就是链表的第一个实际节点
 * 
 * @return struct list_node* 
 */
struct list_node* list_pop(struct list* list) {
    struct list_node* node = list->head.next;
    list_remove(node);
    return node;
}

/**
 * @brief 查找obj_node是否存在
 * 
 */
bool node_find(struct list* list, struct list_node* obj_node) {
    struct list_node* node = list->head.next;
    while (node != &list->tail) {
        if (node == obj_node) {
            return true;
        }
        node = node->next;
    }
    return false;
}

/**
 * @brief 遍历链表，找出第一个让func(node, arg) == true的节点node
 * 
 * @return struct list_node* 
 */
struct list_node* list_traversal(struct list* list, function func, int arg) {
    struct list_node* node = list->head.next;
    if (list_empty(list)) {
        return NULL;
    }
    while (node != &list->tail) {
        if (func(node, arg)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/**
 * @brief 求链表的长度
 * 
 */
uint32_t list_len(struct list* list) {
    struct list_node* node = list->head.next;
    uint32_t len = 0;
    while (node != &list->tail) {
        len++;
        node = node->next;
    }
    return len;
}

/**
 * @brief 判断链表是否为空
 *
 * @return bool 
 */
bool list_empty(struct list* list) {  // 判断队列是否为空
    return (list->head.next == &list->tail ? true : false);
}
