#include <malloc.h>
#include <string.h>
#include "tp_task_queue.h"

void tp_task_queue_submit(tp_task_queue *queue, tp_task task) {
    pthread_mutex_lock(&queue->queue_mutex);
    tp_task_node *node = tp_task_node_create(task);
    if (queue->size == 0) {
        queue->start = node;
        queue->end = node;
    } else if (queue->size >= 1) {
        node->next = queue->start;
        queue->start->prev = node;
        queue->start = node;
    }
    ++queue->size;
    pthread_mutex_unlock(&queue->queue_mutex);
    pthread_cond_signal(&queue->queue_condition);
}

tp_task_node *tp_task_queue_pop(tp_task_queue *queue) {
    pthread_mutex_lock(&queue->queue_mutex);
    tp_task_node *end = queue->end;
    if (queue->size >= 2) {
        end->prev->next = NULL;
        queue->end = end->prev;
    } else if (queue->size == 1) {
        queue->start = NULL;
        queue->end = NULL;
    }
    --queue->size;
    pthread_mutex_unlock(&queue->queue_mutex);
    return end;
}

void tp_task_queue_init(tp_task_queue *task_queue) {
    pthread_mutexattr_init(&task_queue->mutexattr);
    pthread_mutexattr_settype(&task_queue->mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&task_queue->queue_mutex, &task_queue->mutexattr);
    pthread_cond_init(&task_queue->queue_condition, NULL);

    task_queue->size = 0;
    task_queue->start = 0;
    task_queue->end = 0;
}

void tp_task_queue_destroy(tp_task_queue *task_queue) {
    pthread_mutex_destroy(&task_queue->queue_mutex);
    pthread_cond_destroy(&task_queue->queue_condition);
}

void tp_task_queue_node_callback_destroy(tp_task_queue *task_queue, void (*destroy_node_func)(void *)) {
    tp_task_queue_destroy(task_queue);
    tp_task_node *current_node = task_queue->start;
    while (current_node != NULL) {
        tp_task_node *next = current_node->next;
        destroy_node_func(current_node);
        current_node = next;
    }
}

void tp_task_queue_destroy_with_content(tp_task_queue *task_queue) {
    tp_task_queue_node_callback_destroy(task_queue, free);
}

void tp_task_queue_execute(tp_task_queue *queue) {
    tp_task_node *end = tp_task_queue_pop(queue);
    if (end != NULL) {
        tp_task_execute(end->task);
        free(end);
    }
}

void tp_task_queue_submit_action(tp_task_queue *queue, task_action action, void *action_param) {
    tp_task task;
    tp_task_init(&task, action, action_param);
    tp_task_queue_submit(queue, task);
}
