#ifndef TOESOCK_SOCK_UTILS_H
#define TOESOCK_SOCK_UTILS_H

#include <netinet/in.h>
#include "message_utils.h"
#include "meta_info.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct routine_registry_t routine_registry_t;


typedef struct connection_t {
    sockaddr_in sockaddr;
    int socket_fd;
} connection_t;

typedef struct routine_response_t {
    message_t result;
    ret_status_t code;
} routine_response_t;


typedef message_t (*accept_connection_handler_t)(connection_t *connection, message_t *message, command_t cmd);

typedef void(*pre_handler_t)(connection_t *connection, message_t *request);

typedef void(*post_handler_t)(connection_t *connection, message_t *response);

typedef struct thread_params {
    connection_t *connection;
    accept_connection_handler_t conn_handler;
    pre_handler_t pre_handler;
    post_handler_t post_handler;
} thread_params_t;


typedef void(*thread_submitter_t)(void *(*routine)(void *), void *arg);

typedef struct bootstrap_params_t {
    connection_t *server_connection;
    pre_handler_t pre_handler;
    accept_connection_handler_t connection_handler;
    post_handler_t post_handler;
    thread_submitter_t thread_submitter;
} bootstrap_params_t;


void connection_server_init(connection_t *connection, int port_num, int num_listen);

void bootstrap_server(bootstrap_params_t bootstrap_params);

void bootstrap_server_internal(connection_t *serv_connection, accept_connection_handler_t conn_handler,
                               thread_submitter_t subm, pre_handler_t pre_handler, post_handler_t post_handler);

void process_error(int err, char *msg);

message_t
handle_routine_call_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection);

message_t
handle_routine_list_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection);

message_t
handle_routine_meta_info_request(message_t *message, routine_registry_t *routine_registry, connection_t *connection);

routine_response_t call_routine(int sockfd, const char *routine_name, message_t *msg_args);

routine_response_t fetch_routine_meta_info(int sockfd, const char *routine_name);

routine_meta_info_t convert_response_to_routine_meta_info(routine_response_t *response);

void free_routine_response(routine_response_t *routine_response);

#endif //TOESOCK_SOCK_UTILS_H
