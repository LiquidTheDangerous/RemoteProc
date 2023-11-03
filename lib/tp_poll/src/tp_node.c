#include <malloc.h>
#include "tp_node.h"

void tp_task_node_init(tp_task_node *node, tp_task task) {
    node->task = task;
    node->next = NULL;
    node->prev = NULL;
}

tp_task_node *tp_task_create_action_node(task_action action, void *param) {
    tp_task task;
    tp_task_init(&task,action, param);
    return tp_task_node_create(task);
}

tp_task_node * tp_task_node_create(tp_task task) {
    tp_task_node* node = malloc(sizeof(tp_task_node));
    tp_task_node_init(node, task);
    return node;
}
