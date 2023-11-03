#include "message_utils.h"
#include "tp_poll/include/tp_thread_pool.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "remote_routine.h"
#include "sock_utils.h"

connection_t server_connection;
routine_registry_t routine_registry;
tp_thread_pool thread_pool;


message_t command_processor(connection_t *client_connection, message_t *message, command_t cmd) {
    switch (cmd) {
        case CM_ROUTINE_CALL: {
            return handle_routine_call_request(message, &routine_registry, client_connection);
        }
        case CM_ROUTINE_LIST: {
            return handle_routine_list_request(message, &routine_registry, client_connection);
        }
        case CM_FETCH_ROUTINE_META_INFO: {
            return handle_routine_meta_info_request(message, &routine_registry, client_connection);
        }
    }
    message_t empty;
    message_init(&empty, 0);
    return empty;
}

pthread_key_t request_num_key;
pthread_once_t request_num_key_once = PTHREAD_ONCE_INIT;
pthread_mutex_t request_num_mutex;
int request_num = 0;

static void thread_local_val_destroyer(void *data) {
    free(data);
}

static void make_key() {
    pthread_key_create(&request_num_key, thread_local_val_destroyer);
}


void request_pre_handler(connection_t *client_connection, message_t *message) {

    command_t cmd;
    message_peek_last_bytes(message, &cmd, sizeof(cmd), sizeof(cmd));
    if (cmd != CM_ROUTINE_CALL) {
        return;
    }

    pthread_mutex_lock(&request_num_mutex);
    ++request_num;
    pthread_once(&request_num_key_once, make_key);
    void *thread_local_variable = pthread_getspecific(request_num_key);
    if (thread_local_variable == NULL) {
        thread_local_variable = malloc(sizeof(int));
    }
    *(int *) thread_local_variable = request_num;
    pthread_setspecific(request_num_key, thread_local_variable);

    pthread_mutex_unlock(&request_num_mutex);
}

void submit_task(void *(*routine)(void *), void *arg) {
    tp_task_queue_submit_action(&thread_pool.queue, routine, arg);
}

message_t ms_sum(message_t *message, ret_status_t *ret_status) {
    int first = message_pop_int(message);
    int second = message_pop_int(message);
    int r = first + second;
    message_t result;
    message_init(&result, 0);
    message_push_int(&result, r);
    *ret_status = RET_OK;
    printf("result: %d\n", r);
    printf("\n");
    fflush(stdout);

    return result;
}

message_t ms_strcat(message_t *message, ret_status_t *ret_status) {
    message_t result;
    message_init(&result, 0);
    message_byte_array_t first = message_pop_byte_array_new(message);
    message_byte_array_t second = message_pop_byte_array_new(message);

    int total_len = strlen(first.buffer) + strlen(second.buffer);
    char *res = malloc(total_len);
    strcpy(res, first.buffer);
    strcpy(res + strlen(first.buffer), second.buffer);
    message_byte_array_t btr = {.buffer=res, .buff_size = total_len};

    message_push_byte_array(&result, &btr);
    byte_array_free(&first);
    byte_array_free(&second);
    *ret_status = RET_OK;
    return result;
}

pthread_mutex_t intval_mutex;
int val = 0;

message_t ms_shared_intval(message_t *message, ret_status_t *ret_status) {
    message_t resp;
    message_init(&resp, 0);
    pthread_mutex_lock(&intval_mutex);
    message_push_int(&resp, val);
    ++val;
    pthread_mutex_unlock(&intval_mutex);
    return resp;
}

pthread_mutex_t sum_ints_mutex;
int total_sum = 0;

message_t ms_send_int(message_t *message, ret_status_t *ret_status) {
    message_t resp;
    message_init(&resp, 0);
    int client_int = message_pop_int(message);
    pthread_mutex_lock(&sum_ints_mutex);
    total_sum += client_int;
    message_push_int(&resp, total_sum);
    pthread_mutex_unlock(&sum_ints_mutex);
    return resp;
}

message_t get_request_num(message_t *message, ret_status_t *ret_status) {
    message_t response;
    message_init(&response, 0);
    int *req_num_thread_local = pthread_getspecific(request_num_key);
    message_push_int(&response, *req_num_thread_local);
    return response;
}

message_t get_request_num_odd(message_t *message, ret_status_t *ret_status) {
    message_t resp;
    message_init(&resp, 0);
    int *req_num_thread_local = pthread_getspecific(request_num_key);
    message_push_int(&resp, *req_num_thread_local % 2);
    return resp;
}

int main() {
    pthread_mutex_init(&request_num_mutex, NULL);
    pthread_mutex_init(&sum_ints_mutex, NULL);
    pthread_mutex_init(&intval_mutex, NULL);
    tp_init(&thread_pool, 100);
    routine_registry_init(&routine_registry);

    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=ms_sum,
                             .routine_name="sum",
                             .meta_info={
                                     .result_type=argument_int,
                                     .argument_list=routine_args(2, argument_int, argument_int)
                             }
                     }
    );
    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=ms_strcat,
                             .routine_name="strcat",
                             .meta_info={
                                     .result_type=argument_string,
                                     .argument_list=routine_args(2, argument_string, argument_string)
                             }
                     }
    );
    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=ms_shared_intval,
                             .routine_name="get_int",
                             .meta_info={
                                     .result_type=argument_int,
                                     .argument_list=routine_args(0, NULL)
                             }
                     });
    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=get_request_num,
                             .routine_name="get_request_num",
                             .meta_info={
                                     .result_type=argument_int,
                                     .argument_list=routine_args(0, NULL)
                             }
                     });
    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=get_request_num_odd,
                             .routine_name="get_request_num_odd",
                             .meta_info={
                                     .result_type=argument_int,
                                     .argument_list=routine_args(0, NULL)
                             }
                     });
    register_routine(&routine_registry,
                     &(routine_t) {
                             .message_routine=ms_send_int,
                             .routine_name="send_int_to_total_sum",
                             .meta_info={
                                     .result_type=argument_int,
                                     .argument_list=routine_args(1, argument_int)
                             }
                     });
    connection_server_init(&server_connection, 1234, 5);
    bootstrap_server((bootstrap_params_t) {
            .pre_handler=request_pre_handler,
            .post_handler=NULL,
            .thread_submitter=submit_task,
            .connection_handler=command_processor,
            .server_connection=&server_connection
    });
    return 0;
}


