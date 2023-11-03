#include "tp_task.h"


void tp_task_execute(tp_task task) {
    task.task_action(task.task_arg);
}
void tp_task_init(tp_task* task, task_action action, void* task_arg) {
    task->task_action = action;
    task->task_arg = task_arg;
}