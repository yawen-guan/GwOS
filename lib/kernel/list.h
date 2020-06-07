#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H

#include "global.h"

struct list_node {
    struct list_node* prev;
    struct list_node* next;
};

struct list {
    struct list_node head;
    struct list_node tail;
};

typedef bool(function)(struct list_node*, int arg);

/**
 * @brief 初始化环形双向列表
 * 
 */
void list_init(struct list* list);

/**
 * @brief 将节点node插入另一个节点src之前
 * 
 */
void list_insert_before(struct list_node* src, struct list_node* node);

/**
 * @brief 让node成为队头head的下一个节点
 * 
 */
void list_push(struct list* plist, struct list_node* node);

/**
 * @brief 在队尾tail前插入node
 * 
 */
void list_append(struct list* plist, struct list_node* node);
/**
 * @brief 删除node
 * 
 */
void list_remove(struct list_node* node);

/**
 * @brief 弹出head.next, 也就是链表的第一个实际节点
 * 
 * @return struct list_node* 
 */
struct list_node* list_pop(struct list* plist);

/**
 * @brief 查找obj_node是否存在
 * 
 */
bool node_find(struct list* plist, struct list_node* obj_node);

/**
 * @brief 遍历链表，找出第一个让func(node, arg) == true的节点node
 * 
 * @return struct list_node* 
 */
struct list_node* list_traversal(struct list* plist, function func, int arg);

/**
 * @brief 求链表的长度
 * 
 */
uint32_t list_len(struct list* plist);

/**
 * @brief 判断链表是否为空
 *
 * @return bool 
 */
bool list_empty(struct list* plist);

#endif