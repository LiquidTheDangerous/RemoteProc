
#ifndef THREAD_POOL_TASK
#define THREAD_POOL_TASK

typedef void*(*task_action)(void* task_arg);

typedef struct tp_task {
    void* task_arg;
    task_action task_action;
} tp_task;

void tp_task_execute(tp_task task);

void tp_task_init(tp_task* task, task_action action, void* task_arg);

#endif //THREAD_POOL_TASK