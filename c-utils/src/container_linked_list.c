/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <stdlib.h>
#include "container_linked_list.h"

linked_node create_linked_node() {
    linked_node node;
    node.pre = NULL;
    node.next = NULL;
    return node;
}

linked_list create_linked_list(linked_node_compare compare) {
    linked_list temp_list;
    temp_list.linked_size = 0;
    temp_list.head = NULL;
    temp_list.tail = NULL;
    temp_list.compare = compare;
    return temp_list;
}

FUNCTION_IRAM linked_list create_linked_list_safe(linked_node_compare compare)
{
    linked_list temp_list;
    temp_list.linked_size = 0;
    temp_list.head = NULL;
    temp_list.tail = NULL;
    temp_list.compare = compare;
    pthread_mutex_init(&temp_list.mutex, NULL);
    return temp_list;
}

void linked_list_insert_front(linked_list *link_list, linked_node *node) {
    if (link_list == NULL || NULL == node)
        return;

    node->next = link_list->head;
    node->pre = NULL;
    link_list->head = node;

    if (NULL != node->next) {
        node->next->pre = node;
    }

    if (NULL == link_list->tail)
        link_list->tail = node;

    link_list->linked_size++;
}

void linked_list_insert_front_safe(linked_list *link_list, linked_node *node) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_insert_front(link_list, node);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_insert_tail(linked_list *link_list, linked_node *node) {
    if (NULL == link_list || NULL == node)
        return;

    node->pre = link_list->tail;
    node->next = NULL;
    link_list->tail = node;

    if (NULL != node->pre) {
        node->pre->next = node;
    }

    if (NULL == link_list->head) {
        link_list->head = node;
    }

    link_list->linked_size++;
}

void linked_list_insert_tail_safe(linked_list *link_list, linked_node *node) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_insert_tail(link_list, node);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_insert_node_before(linked_list *link_list, linked_node *nodeto,
                                    linked_node *nodebe) {
    if (NULL == link_list || NULL == nodeto || NULL == nodebe)
        return;

    if (link_list->head == nodeto)
        link_list->head = nodebe;

    nodebe->pre = nodeto->pre;
    if (NULL != nodebe->pre)
        nodebe->pre->next = nodebe;
    nodebe->next = nodeto;

    nodeto->pre = nodebe;
    link_list->linked_size++;
}

void linked_list_insert_node_before_safe(linked_list *link_list,
                                         linked_node *nodeto,
                                         linked_node *nodebe) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_insert_node_before(link_list, nodeto, nodebe);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_insert_node_later(linked_list *link_list, linked_node *node_to,
                                   linked_node *node_be) {
    if (NULL == link_list || NULL == node_to || NULL == node_be)
        return;

    if (link_list->tail == node_to)
        link_list->tail = node_be;

    node_be->next = node_to->next;
    if (NULL != node_be->next)
        node_be->next->pre = node_be;
    node_be->pre = node_to;

    node_to->next = node_be;
    link_list->linked_size++;
}

void linked_list_insert_node_later_safe(linked_list *link_list,
                                        linked_node *node_to,
                                        linked_node *node_be) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_insert_node_later(link_list, node_to, node_be);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_delete_node(linked_list *link_list, linked_node *to_delete) {
    if (NULL == link_list || NULL == to_delete)
        return;

    if (link_list->head == to_delete)
        link_list->head = to_delete->next;

    if (link_list->tail == to_delete)
        link_list->tail = to_delete->pre;

    if (NULL != to_delete->next)
        to_delete->next->pre = to_delete->pre;

    if (NULL != to_delete->pre)
        to_delete->pre->next = to_delete->next;

    to_delete->next = NULL;
    to_delete->pre = NULL;
    link_list->linked_size--;
}

void linked_list_delete_node_safe(linked_list *link_list,
                                  linked_node *to_delete) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_delete_node(link_list, to_delete);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_insert_sort(linked_list *link_list, linked_node *to_insert) {
    if (NULL == link_list || NULL == to_insert)
        return;

    /**
     * @brief Find the first node that is smaller than the inserted node
     * If no comparison function is specified, insert directly to the end
     */
    linked_node *current = link_list->head;
    while (NULL != current) {
        if (NULL == link_list->compare ||
            link_list->compare(to_insert, current) > 0) {
            break;
        }
        current = current->next;
    }

    /**
     * @brief  Not found, insert at the end
     * If found, insert before the found node
     */
    if (NULL == current) {
        linked_list_insert_tail(link_list, to_insert);
    } else {
        linked_list_insert_node_before(link_list, current, to_insert);
    }
}

void linked_list_insert_sort_safe(linked_list *link_list,
                                  linked_node *to_insert) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_insert_sort(link_list, to_insert);
    pthread_mutex_unlock(&link_list->mutex);
}

linked_node *pop_linked_list_head(linked_list *link_list) {
    linked_node *node = NULL;
    if (NULL != link_list) {
        node = link_list->head;
        linked_list_delete_node(link_list, node);
    }
    return node;
}

linked_node *pop_linked_list_head_safe(linked_list *link_list) {
    linked_node *node = NULL;
    pthread_mutex_lock(&link_list->mutex);
    node = pop_linked_list_head(link_list);
    pthread_mutex_unlock(&link_list->mutex);
    return node;
}

linked_node *pop_linked_list_tail(linked_list *link_list) {
    linked_node *node = NULL;
    if (NULL != link_list) {
        node = link_list->tail;
        linked_list_delete_node(link_list, node);
    }
    return node;
}

linked_node *pop_linked_list_tail_safe(linked_list *link_list) {
    linked_node *node = NULL;
    pthread_mutex_lock(&link_list->mutex);
    node = pop_linked_list_tail(link_list);
    pthread_mutex_unlock(&link_list->mutex);
    return node;
}

linked_node *linked_list_find_node(linked_list *link_list, find_node find,
                                   void *args) {
    if (NULL == link_list || NULL == find)
        return NULL;

    linked_node *current = link_list->head;
    while (NULL != current) {
        if (1 == find(current, args)) {
            break;
        }
        current = current->next;
    }

    return current;
}

linked_node *linked_list_find_node_safe(linked_list *link_list, find_node find,
                                        void *args) {
    if (NULL == link_list || NULL == find)
        return NULL;

    pthread_mutex_lock(&link_list->mutex);
    linked_node *node = linked_list_find_node(link_list, find, args);
    pthread_mutex_unlock(&link_list->mutex);
    return node;
}

void linked_list_for_each_forward(linked_list *link_list, deal_node deal,
                                  void *args) {
    if (NULL == link_list || NULL == deal)
        return;

    linked_node *current = link_list->head;
    while (NULL != current) {
        deal(current, args);
        current = current->next;
    }
}

void linked_list_for_each_forward_safe(linked_list *link_list, deal_node deal,
                                       void *args) {
    if (NULL == link_list || NULL == deal)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_for_each_forward(link_list, deal, args);
    pthread_mutex_unlock(&link_list->mutex);
}

void linked_list_for_each_back(linked_list *link_list, deal_node deal,
                               void *args) {
    if (NULL == link_list || NULL == deal)
        return;

    linked_node *current = link_list->tail;
    while (NULL != current) {
        deal(current, args);
        current = current->pre;
    }
}

void linked_list_for_each_back_safe(linked_list *link_list, deal_node deal,
                                    void *args) {
    if (NULL == link_list || NULL == deal)
        return;

    pthread_mutex_lock(&link_list->mutex);
    linked_list_for_each_back(link_list, deal, args);
    pthread_mutex_unlock(&link_list->mutex);
}

void destroy_linked_list(linked_list *link_list) {
    if (NULL == link_list)
        return;

    linked_node *current = link_list->head;
    linked_node *tofree = link_list->head;
    while (NULL != current) {
        current = current->next;
        if (NULL != tofree)
            fast_free(tofree);
        tofree = current;
    }

    link_list->linked_size = 0;
    link_list->head = NULL;
    link_list->tail = NULL;
}

void destroy_linked_list_safe(linked_list *link_list) {
    if (NULL == link_list)
        return;

    pthread_mutex_lock(&link_list->mutex);
    destroy_linked_list(link_list);
    pthread_mutex_unlock(&link_list->mutex);
}
