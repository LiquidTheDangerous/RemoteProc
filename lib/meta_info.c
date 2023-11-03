#include "meta_info.h"
#include "message_utils.h"
#include "sock_utils.h"
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void routine_realloc_internal(routine_argument_list_t *args, size_t new_size) {
    if (args->buff_size + new_size >= args->buff_cap) {
        args->buff_cap += args->buff_size + new_size;
        args->buff_cap *= 2;
        void *new_buff = malloc(args->buff_cap);
        memcpy(new_buff, args->args, args->buff_cap);
        args->args = new_buff;
    }
}

void routine_argument_list_init(routine_argument_list_t *lst, size_t cap) {
    bzero(lst, sizeof(*lst));
    lst->count = 0;
    cap = cap * sizeof(routine_argument_t);
    lst->args = malloc(cap);
    lst->buff_cap = cap;
    bzero(lst->args, cap);
}

void routine_argument_list_push(routine_argument_list_t *lst, routine_argument_t argument) {
    routine_realloc_internal(lst, sizeof(argument));
    lst->args[lst->count] = argument;
    lst->buff_size += sizeof(argument);
    ++lst->count;
}

routine_argument_list_t routine_args(size_t count, ...) {
    routine_argument_list_t lst;
    routine_argument_list_init(&lst, count);
    va_list arg;
    va_start(arg, count);
    int i;
    for (i = 0; i < count; ++i) {
        routine_argument_t rarg = va_arg(arg, routine_argument_t);
        routine_argument_list_push(&lst, rarg);
    }
    va_end(arg);
    return lst;
}

void print_routine_meta_info(FILE *file, routine_meta_info_t *meta_info) {
    printf("result type: ");
    print_arg(file, meta_info->result_type, 1);
    printf("args count: %ld\n", meta_info->argument_list.count);
    int i;
    for (i = 0; i < meta_info->argument_list.count; ++i) {
        printf("arg_%d: ", i);
        print_arg(file, meta_info->argument_list.args[i], 1);
    }
}

void print_arg(FILE *file, routine_argument_t argument, int line_feed) {
    char lf = line_feed == 0 ? '\0' : '\n';
    switch (argument.arg_type) {
        case ARG_LONG:
            fprintf(file, "long%c", lf);
            break;
        case ARG_INT:
            fprintf(file, "int%c", lf);
            break;
        case ARG_SHORT:
            fprintf(file, "short%c", lf);
            break;
        case ARG_BYTE:
            fprintf(file, "byte%c", lf);
            break;
        case ARG_BYTEARRAY:
            fprintf(file, "bytearray%c", lf);
            break;
        case ARG_VOID:
            fprintf(file, "void%c", lf);
            break;
        case ARG_STRING:
            fprintf(file, "string%c", lf);
            break;
    }
}

void read_long(FILE *file, long *value) {
    char buff[32];
    fgets(buff, 32, file);
    *value = strtol(buff, NULL, 10);
}

void read_int(FILE *file, int *value) {
    read_long(file, (long *) value);
}

void read_short(FILE *file, short *value) {
    read_long(file, (long *) value);
}

void read_byte(FILE *file, byte_t *value) {
    read_long(file, (long *) value);
}

void read_char(FILE *file, char *value) {
    read_long(file, (long *) value);
}

void read_bytearray(FILE *file, byte_t *bytearray, int size) {
    fgets((char *) bytearray, size, file);
}

void read_string(FILE *file, char *string, int size) {
    fscanf(file, "%s", string);
//    fgets(string, size, file);
}

void interactive_routine_call(char *routine_name, int sockfd) {
    routine_response_t resp = fetch_routine_meta_info(sockfd, routine_name);
    if (resp.code == RET_OK) {
        routine_meta_info_t meta = convert_response_to_routine_meta_info(&resp);
        print_routine_meta_info(stdout, &meta);

        message_t tmp;
        message_init(&tmp, 0);


        int i;
        for (i = 0; i < meta.argument_list.count; ++i) {
            printf("arg_%d(", i);
            print_arg(stdout, meta.argument_list.args[i], 0);
            printf("): ");
            read_meta_depend(meta.argument_list.args[i], &tmp);
        }

        message_t args;
        message_init(&args, tmp.buff_cap);
        for (i = (int) meta.argument_list.count - 1; i >= 0; --i) {
            routine_argument_t *arg = (meta.argument_list.args + i);
            if (arg->arg_type != ARG_STRING && arg->arg_type != ARG_BYTEARRAY) {
                char data[32];
                message_pop(&tmp, data, (int) arg->arg_size);
                message_push(&args, data, (int) arg->arg_size);
            } else {
                message_byte_array_t bt = message_pop_byte_array(&tmp);
                message_push_byte_array(&args, &bt);
            }
        }

        routine_response_t ret = call_routine(sockfd, routine_name, &args);
        if (ret.code == RET_OK) {
            printf("resp: ");
            fflush(stdout);
            write_meta_depend(meta.result_type, &ret.result);
            fflush(stdout);
            printf("\n");
        } else {
            printf("something went wrong...\n");
        }
        message_free(&tmp);
        message_free(&args);
        free_routine_response(&ret);
        free_routine_response(&resp);
    } else {
        printf("something went wrong...\n");
    }
}

void read_meta_depend(routine_argument_t arg, message_t *message) {
    switch (arg.arg_type) {
        case ARG_LONG: {
            long val;
            read_long(stdin, &val);
            message_push_long(message, val);
            break;
        }
        case ARG_INT: {
            int val;
            read_int(stdin, &val);
            message_push_int(message, val);
            break;
        }
        case ARG_SHORT: {
            short val;
            read_short(stdin, &val);
            message_push_short(message, val);
            break;
        }
        case ARG_BYTE: {
            byte_t val;
            read_byte(stdin, &val);
            message_push_byte(message, val);
            break;
        }
        case ARG_CHAR1: {
            char val;
            read_char(stdin, &val);
            message_push_char(message, val);
            break;
        }
        case ARG_BYTEARRAY: {
            char bytes[256];
            read_bytearray(stdin, (byte_t *) bytes, 256);
            message_push_byte_array(message, &(message_byte_array_t) {.buffer=bytes, .buff_size=strlen(bytes)});
            break;
        }
        case ARG_STRING: {
            char bytes[256];
            read_string(stdin, bytes, 256);
            message_push_byte_array(message, &(message_byte_array_t) {.buffer=bytes, .buff_size=strlen(bytes)});
            break;
        }
        case ARG_VOID:
            break;
    }
}

void write_long(FILE *file, long value) {
    fprintf(file, "%ld", value);
}

void write_int(FILE *file, int value) {
    write_long(file, value);
}

void write_short(FILE *file, short value) {
    write_long(file, value);
}

void write_byte(FILE *file, byte_t value) {
    write_long(file, value);
}

void write_char(FILE *file, char value) {
    write_long(file, value);
}

void write_bytearray(FILE *file, byte_t *bytearray, int size) {
    write(fileno(file), bytearray, size);
}

void write_string(FILE *file, char *string, int size) {
    write(fileno(file), string, size);
}

void write_meta_depend(routine_argument_t arg, message_t *message) {
    switch (arg.arg_type) {
        case ARG_LONG: {
            long val = message_pop_long(message);
            write_long(stdout, val);
            break;
        }
        case ARG_INT: {
            int val = message_pop_int(message);
            write_int(stdout, val);
            break;
        }
        case ARG_SHORT: {
            short val = message_pop_short(message);
            write_short(stdout, val);
            break;
        }
        case ARG_BYTE: {
            byte_t val = message_pop_byte(message);
            write_byte(stdout, val);
            break;
        }
        case ARG_CHAR1: {
            char val = message_pop_char(message);
            write_char(stdout, val);
            break;
        }
        case ARG_BYTEARRAY: {
            message_byte_array_t bt = message_pop_byte_array(message);
            write_bytearray(stdout, bt.buffer, bt.buff_size);
            break;
        }
        case ARG_STRING: {
            message_byte_array_t bt = message_pop_byte_array(message);
            write_string(stdout, (char *) bt.buffer, bt.buff_size);
            break;
        }
        case ARG_VOID:
            break;
    }
}
