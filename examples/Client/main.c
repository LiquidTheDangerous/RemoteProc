#include "message_utils.h"
#include "sock_utils.h"
#include "remote_routine.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

void replace_chars(char *source, char target, char replacement) {
    while (*source) {
        if (*source == target) {
            *source = replacement;
        }
        ++source;
    }
}

int main() {
    sockaddr_in serv_addr;
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(1234);
    serv_addr.sin_family = AF_INET;
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    int connect_res = connect(client_fd, (sockaddr *) (&serv_addr), sizeof(serv_addr));
    process_error(connect_res, "failed to connect");
    char cmd[255];
    printf("type help to see list of available commands\n");
    while (1) {
        printf(">>");
        fflush(stdout);
        fgets(cmd, 255, stdin);
        replace_chars(cmd, 0xA, 0x0);
        if (strcmp(cmd, "help") == 0) {
            printf("RoutineList - list of available remote routines\n");
            printf("MetaInfo <routine name> - routine meta info\n");
            printf("call <routine name> - calls routine by name\n");
            printf("quit - exit rpc cli\n");
        } else if (strcmp(cmd, "RoutineList") == 0) {
            message_t args;
            message_init(&args, 0);
            message_push_cmd(&args, CM_ROUTINE_LIST);
            message_serialize_fd(&args, client_fd);
            message_t res = message_deserialize_fd_new(client_fd);
            command_t cm = message_pop_command(&res);
            if (cm == RET_OK) {
                print_routine_registry(stdout, &res);
            }
            message_free(&res);
            message_free(&args);
        } else if (strncmp(cmd, "call", 4) == 0) {
            char *routine_name = cmd + 5;
            interactive_routine_call(routine_name, client_fd);
        } else if (strncmp(cmd, "MetaInfo", 8) == 0) {
            char *routine_name = cmd + 9;
            routine_response_t resp = fetch_routine_meta_info(client_fd, routine_name);
            routine_meta_info_t meta = convert_response_to_routine_meta_info(&resp);
            print_routine_meta_info(stdout, &meta);
            free_routine_response(&resp);
        } else if (strcmp(cmd, "quit") == 0) {
            message_t request;
            message_init(&request, 0);
            message_push_cmd(&request, CM_CLOSE);
            message_serialize_fd(&request, client_fd);
            message_free(&request);
        }
    }
    return 0;
}