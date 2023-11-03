#ifndef NICESOCK_TP_THREAD_POOL_H
#define NICESOCK_TP_THREAD_POOL_H

#include "tp_task_queue.h"

typedef struct tp_thread_pool {
    tp_task_queue queue;
    pthread_t *threads;
    int num_threads;
    int is_pool_tasks;
} tp_thread_pool;

void tp_init(tp_thread_pool *pool, int thread_num);

void tp_join_all(tp_thread_pool *pool);

void tp_destroy(tp_thread_pool *pool);

#endif //NICESOCK_TP_THREAD_POOL_H
