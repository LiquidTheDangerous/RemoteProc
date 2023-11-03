#ifndef NICESOCK_TP_TASK_QUEUE_H
#define NICESOCK_TP_TASK_QUEUE_H

#include "tp_node.h"
#include <pthread.h>

typedef struct tp_task_queue {
    tp_task_node *start;
    tp_task_node *end;
    int size;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_condition;
    pthread_mutexattr_t mutexattr;
} tp_task_queue;

void tp_task_queue_init(tp_task_queue *task_queue);

void tp_task_queue_destroy(tp_task_queue *task_queue);

void tp_task_queue_node_callback_destroy(tp_task_queue *task_queue, void(*destroy_node_func)(void *node));

void tp_task_queue_destroy_with_content(tp_task_queue *task_queue);

void tp_task_queue_submit(tp_task_queue *queue, tp_task task);

void tp_task_queue_submit_action(tp_task_queue *queue, task_action action, void *action_param);

void tp_task_queue_execute(tp_task_queue *queue);

tp_task_node *tp_task_queue_pop(tp_task_queue *queue);


#endif //NICESOCK_TP_TASK_QUEUE_H
