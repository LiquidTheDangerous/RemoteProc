#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sock_utils.h"
#include "message_utils.h"
#include "btree.h"
#include "remote_routine.h"


void connection_server_init(connection_t *connection, int port_num, int num_listen) {
    bzero(connection, sizeof(connection_t));
    connection->sockaddr.sin_addr.s_addr = INADDR_ANY;
    connection->sockaddr.sin_port = htons(port_num);
    connection->sockaddr.sin_family = AF_INET;
    connection->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    process_error(connection->socket_fd, "unable to create socket\n");

    int bind_result = bind(connection->socket_fd,
                           (struct sockaddr *) &connection->sockaddr,
                           sizeof(connection->sockaddr));

    process_error(bind_result, "unable to bind interface\n");

    int listen_result = listen(connection->socket_fd, num_listen);
    process_error(listen_result, "unable to listen with given num_listen");
}

void process_error(int err, char *msg) {
    if (err < 0) {
        perror(msg);
        exit(0);
    }
}

connection_t *accept_connection_internal(connection_t *serv_connection) {
    connection_t *incoming = calloc(1, sizeof(connection_t));
    socklen_t addr_len = sizeof(incoming->sockaddr);
    int new_sock_fd = accept(serv_connection->socket_fd, (sockaddr *) incoming, &addr_len);
    incoming->socket_fd = new_sock_fd;
    return incoming;
}


void *thread_routine_internal(void *arg) {
    thread_params_t *params = arg;
    command_t cmd;
    do {
        message_t message = message_deserialize_fd_new(params->connection->socket_fd);
        if (params->pre_handler)
            params->pre_handler(params->connection, &message);
        cmd = message_pop_command(&message);
        message_t response = params->conn_handler(params->connection, &message, cmd);
        if (params->post_handler)
            params->post_handler(params->connection, &response);
        message_serialize_fd(&response, params->connection->socket_fd);
        message_free(&response);
        message_free(&message);
    } while (cmd != CM_CLOSE);
    close(params->connection->socket_fd);
    free(params->connection);
    free(params);
    return NULL;
}


void bootstrap_server_internal(connection_t *serv_connection, accept_connection_handler_t conn_handler,
                               thread_submitter_t subm, pre_handler_t pre_handler, post_handler_t post_handler) {
    for (;;) {
        connection_t *new_connection = accept_connection_internal(serv_connection);
        thread_params_t *tp = malloc(sizeof(thread_params_t));
        tp->pre_handler = pre_handler;
        tp->post_handler = post_handler;
        tp->connection = new_connection;
        tp->conn_handler = conn_handler;
        subm(thread_routine_internal, tp);
    }
}

message_t
handle_routine_call_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection) {
    message_t result;
    message_byte_array_t routine_name = message_pop_byte_array_new(message);
    routine_t *routine = find_routine(routine_registry, (char *) routine_name.buffer);
    if (routine == NULL) {
        message_init(&result, 0);
        message_push_status(&result, RET_ROUTINE_NOT_FOUND);
        goto exit;
    }
    ret_status_t ret_status = RET_OK;
    result = routine->message_routine(message, &ret_status);
    message_push_status(&result, ret_status);
    exit:
    byte_array_free(&routine_name);
    return result;
}

routine_response_t call_routine(int sockfd, const char *routine_name, message_t *msg_args) {
    message_push_byte_array(msg_args, &(message_byte_array_t) {.buffer=routine_name, .buff_size=strlen(routine_name)});
    message_push_cmd(msg_args, CM_ROUTINE_CALL);
    message_serialize_fd(msg_args, sockfd);
    message_t result = message_deserialize_fd_new(sockfd);
    ret_status_t status = message_pop_status(&result);
    return (routine_response_t) {.result=result, .code=status};
}

void free_routine_response(routine_response_t *routine_response) {
    message_free(&routine_response->result);
}

bool iterate_internal(const void *node, void *data) {
    const routine_t *routine = node;
    message_t *mesg = data;
    message_push_byte_array(mesg, &(message_byte_array_t) {.buffer=routine->routine_name, .buff_size=strlen(
            routine->routine_name)});
    return true;
}

message_t
handle_routine_list_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection) {
    struct btree *bt = (struct btree *) routine_registry->handle;
    message_t result;
    message_init(&result, 0);
    btree_ascend(bt, NULL, iterate_internal, &result);
    message_push_int(&result, btree_count(bt));
    message_push_status(&result, RET_OK);
    return result;
}

message_t
handle_routine_meta_info_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection) {
    message_t response;
    message_init(&response, 0);
    message_byte_array_t routine_name = message_pop_byte_array_new(message);
    struct btree *bt = (struct btree *) routine_registry->handle;
    routine_t *routine = btree_get(bt, &(struct routine_t) {.routine_name=(char *) routine_name.buffer});
    if (routine == NULL) {
        message_push_status(&response, RET_ROUTINE_NOT_FOUND);
        goto defer;
    }
    int i;
    for (i = (int) routine->meta_info.argument_list.count - 1; i >= 0; --i) {
        message_push(&response,
                     &(routine->meta_info.argument_list.args[i]),
                     sizeof((routine->meta_info.argument_list.args[i])));
    }
    message_push(&response, &routine->meta_info.argument_list.count, sizeof(routine->meta_info.argument_list.count));
    message_push(&response, &routine->meta_info.result_type, sizeof(routine->meta_info.result_type));
    message_push_status(&response, RET_OK);
    defer:
    return response;
}

routine_response_t fetch_routine_meta_info(int sockfd, const char *routine_name) {
    routine_response_t response;
    message_t request;
    message_init(&request, 0);
    message_push_byte_array(&request, &(message_byte_array_t) {.buffer=routine_name, .buff_size=strlen(routine_name)});
    message_push_cmd(&request, CM_FETCH_ROUTINE_META_INFO);
    message_serialize_fd(&request, sockfd);
    message_t m_resp = message_deserialize_fd_new(sockfd);
    ret_status_t status = message_pop_status(&m_resp);
    response = (routine_response_t) {.result=m_resp, .code=status};
    return response;
}

routine_meta_info_t convert_response_to_routine_meta_info(routine_response_t *response) {
    routine_meta_info_t res;
    message_pop(&response->result, &res.result_type, sizeof(res.result_type));
    message_pop(&response->result, &res.argument_list.count, sizeof(res.argument_list.count));
    routine_argument_list_t arg_list;
    routine_argument_list_init(&arg_list, res.argument_list.count);

    int i;
    for (i = 0; i < res.argument_list.count; ++i) {
        routine_argument_t arg;
        message_pop(&response->result, &arg, sizeof(arg));
        routine_argument_list_push(&arg_list, arg);
    }
    res.argument_list = arg_list;
    return res;
}

void bootstrap_server(bootstrap_params_t bootstrap_params) {
    bootstrap_server_internal(bootstrap_params.server_connection,
                              bootstrap_params.connection_handler,
                              bootstrap_params.thread_submitter,
                              bootstrap_params.pre_handler,
                              bootstrap_params.post_handler);
}
