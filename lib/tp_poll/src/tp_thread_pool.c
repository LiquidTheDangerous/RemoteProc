#include <malloc.h>
#include <string.h>
#include "tp_thread_pool.h"


void *start_thread_internal(void *p) {
    tp_thread_pool *pool = p;
    while (1) {

        pthread_mutex_lock(&pool->queue.queue_mutex);
        while (pool->queue.size == 0) {
            pthread_cond_wait(&pool->queue.queue_condition, &pool->queue.queue_mutex);
        }

        tp_task_node *node = tp_task_queue_pop(&pool->queue);

        pthread_mutex_unlock(&pool->queue.queue_mutex);
        if (node != NULL) {
            tp_task_execute(node->task);
            free(node);
        }
        pthread_cond_signal(&pool->queue.queue_condition);
    }
}

void tp_init(tp_thread_pool *pool, int thread_num) {
    tp_task_queue_init(&pool->queue);
    pthread_t *threads = malloc(thread_num * sizeof(pthread_t));
    pool->num_threads = thread_num;
    pool->threads = threads;
    int i;
    for (i = 0; i < thread_num; ++i) {
        if (pthread_create(&pool->threads[i], NULL, &start_thread_internal, pool) != 0) {
            perror("Failed to create the thread");
        }
    }
    pool->is_pool_tasks = 1;
}

void tp_destroy(tp_thread_pool *pool) {
    tp_task_queue_destroy_with_content(&pool->queue);
    int i;
    for (i = 0; i < pool->num_threads; ++i) {
        pthread_cancel(pool->threads[i]);
    }
    free(pool->threads);
}

void tp_join_all(tp_thread_pool *pool) {
    int i = 0;
    for (i = 0; i < pool->num_threads; ++i) {
        pthread_join(pool->threads[i], NULL);
    }
}


