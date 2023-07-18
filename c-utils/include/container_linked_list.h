/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CONTAINER_LINKED_LIST_H
#define CONTAINER_LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <allocator.h>

/**
 * linked node
 */
typedef struct linked_node_ {
    /**
     * pre  node
     */
    struct linked_node_ *pre;

    /**
     * next node
     */
    struct linked_node_ *next;
} linked_node;


/**
 * Node comparison function
 */
typedef int (*linked_node_compare)(void *a, void *b);

/**
 * Node processing function
 */
typedef void (*deal_node)(linked_node *node, void *args);

/**
 * Node search function, returns 1 to indicate that the corresponding node is the node that needs to be found
 */
typedef int (*find_node)(void *node, void *args);


/**
 * @brief  Create linked node
 * @note
 * @retval
 */
FUNCTION_IRAM linked_node create_linked_node();

/**
 * Linked list
 */
typedef struct {
    /**
     * node count
     */
    int linked_size;

    /**
     * head node
     */
    linked_node *head;

    /**
     * tail node
     */
    linked_node *tail;

    /**
     * compare function
     */
    linked_node_compare compare;

    /**
     * Linked list access mutex
     * Supports thread-safe operations
     */
    pthread_mutex_t mutex;
} linked_list;

/**
 * @brief  Create linked list, initialize
 * @note
 * @param  compare:
 * @retval
 */
FUNCTION_IRAM linked_list create_linked_list(linked_node_compare compare);

/**
 * @brief  Create thread safe linked list, initialize
 * @note
 * @param  compare:
 * @retval
 */
FUNCTION_IRAM linked_list create_linked_list_safe(linked_node_compare compare);

/**
 * @brief  Insert node from head
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
FUNCTION_IRAM  void linked_list_insert_front(linked_list *link_list, linked_node *node);

/**
 * @brief  Insert node from head, safe insertion
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_front_safe(linked_list *link_list, linked_node *node);

/**
 * @brief  Insert node from tail
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
FUNCTION_IRAM void linked_list_insert_tail(linked_list *link_list, linked_node *node);

/**
 * @brief  Insert node from tail, thread safe
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_tail_safe(linked_list *link_list, linked_node *node);

/**
 * @brief  Insert before node nodeto
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
FUNCTION_IRAM void linked_list_insert_node_before(linked_list *link_list, linked_node *nodeto,
                                    linked_node *nodebe);

/**
 * @brief  Inserted before node nodeto, thread safe
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_before_safe(linked_list *link_list,
                                         linked_node *nodeto,
                                         linked_node *nodebe);

/**
 * @brief  Insert after node node_to
 * @note
 * @param  *node_to:
 * @param  node_be:
 * @retval None
 */
FUNCTION_IRAM void linked_list_insert_node_later(linked_list *link_list, linked_node *node_to, linked_node *node_be);

/**
 * @brief  Insert after node node_to, thread safe
 * @note
 * @param  *node_to:
 * @param  *node_be:
 * @retval None
 */
void linked_list_insert_node_later_safe(linked_list *link_list, linked_node *node_to, linked_node *node_be);

/**
 * @brief  Removes the specified node from the linked list
 * @note
 * @param  *link_list:
 * @param  *to_delete:
 * @retval None
 */
FUNCTION_IRAM void linked_list_delete_node(linked_list *link_list, linked_node *to_delete);

/**
 * @brief  Removes the specified node from the linked list, thread safe
 * @note
 * @param  *link_list:
 * @param  *to_delete:
 * @retval None
 */
void linked_list_delete_node_safe(linked_list *link_list,linked_node *to_delete);

/**
 * @brief  Insert sort by compare
 * @note
 * @param  *link_list:
 * @param  *to_insert:
 * @retval None
 */
FUNCTION_IRAM void linked_list_insert_sort(linked_list *link_list, linked_node *to_insert);

/**
 * @brief  Ordered insertion, the original inserted will be sorted according to the specified compare, thread safe
 * @note
 * @param  *link_list:
 * @param  *to_insert:
 * @retval None
 */
void linked_list_insert_sort_safe(linked_list *link_list, linked_node *to_insert);

/**
 * @brief  Take the head node of the linked list and remove it from the linked list
 * @note
 * @param  link_list:
 * @retval
 */
FUNCTION_IRAM linked_node *pop_linked_list_head(linked_list *link_list);

/**
 * @brief  Take the head node of the linked list and remove it from the linked list, thread safe
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_head_safe(linked_list *link_list);

/**
 * @brief   Remove the tail node of the linked list and delete it from the linked list
 * @note
 * @param  link_list:
 * @retval
 */
FUNCTION_IRAM linked_node *pop_linked_list_tail(linked_list *link_list);

/**
 * @brief  Remove the tail node of the linked list and delete it from the linked list, thread safe
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_tail_safe(linked_list *link_list);

/**
 * @brief  Find the specified node
 * @note  find returns 1 to indicate that the node is needed, the first node from the previous
 * @param  *link_list:
 * @param  find:
 * @param  *args:
 * @retval None
 */
FUNCTION_IRAM linked_node *linked_list_find_node(linked_list *link_list, find_node find, void *args);

/**
* @brief  Find the specified node, thread safe
* @note   find returns 1 to indicate that the node is needed, the first node from the previous
* @param  *link_list:
* @param  find:
* @param  *args:
* @retval None
*/
linked_node *linked_list_find_node_safe(linked_list *link_list, find_node find, void *args);

/**
 * @brief  Perform the specified operation on each node, from front to back
 * @note   You should not operate the linked list in the processing function,
 *          you cannot delete and add, you should only access the node content
 * @param  *link_list:
 * @param  deal:
 * @param *args Custom parameters
 * @retval None
 */
FUNCTION_IRAM void linked_list_for_each_forward(linked_list *link_list, deal_node deal, void *args);

/**
 * @brief  Perform the specified operation on each node, from front to back, thread sage
 * @note   You should not operate the linked list in the processing function,
 *          you cannot delete and add, you should only access the node content
 * @param  *link_list:
 * @param  deal:
 * @param *args Custom parameters
 * @retval None
 */
void linked_list_for_each_forward_safe(linked_list *link_list, deal_node deal, void *args);

/**
 * @brief  Perform the specified operation on each node, from back to front
 * @note   You should not operate the linked list in the processing function,
 *          you cannot delete and add, you should only access the node content
 * @param  *link_list:
 * @param  deal:
 * @param *args Custom parameters
 * @retval None
 */
FUNCTION_IRAM void linked_list_for_each_back(linked_list *link_list, deal_node deal, void *args);

/**
 * @brief  Perform the specified operation on each node, from back to front, thread safe
 * @note You should not operate the linked list in the processing function,
 *          you cannot delete and add, you should only access the node content
 * @param  *link_list:
 * @param  deal:
 * @param *args Custom parameters
 * @retval None
 */
void linked_list_for_each_back_safe(linked_list *link_list, deal_node deal,
                                    void *args);

/**
 * @brief  Release all nodes, all nodes are created through heap memory
 * @note
 * @param  *link_list:
 * @retval None
 */
FUNCTION_IRAM void destroy_linked_list(linked_list *link_list);

/**
 * @brief  Release all nodes, all nodes are created through heap memory, thread-safe
 * @note
 * @param  *link_list:
 * @retval None
 */
void destroy_linked_list_safe(linked_list *link_list);

#ifdef __cplusplus
}
#endif

#endif /* CONTAINER_LINKED_LIST_H */
