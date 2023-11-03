#ifndef NICESOCK_TP_NODE_H
#define NICESOCK_TP_NODE_H

#include "tp_task.h"

typedef struct tp_task_node {
    tp_task task;
    struct tp_task_node *next;
    struct tp_task_node *prev;
} tp_task_node;

void tp_task_node_init(tp_task_node *node, tp_task task);

tp_task_node * tp_task_node_create(tp_task task);

tp_task_node * tp_task_create_action_node(task_action action, void* param);

#endif //NICESOCK_TP_NODE_H
