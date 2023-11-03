#ifndef REMOTEPROCEDURECALL_META_INFO_H
#define REMOTEPROCEDURECALL_META_INFO_H

#include <stddef.h>
#include <stdio.h>
#include "message_utils.h"

typedef enum argument_type_t {
    ARG_LONG,
    ARG_INT,
    ARG_SHORT,
    ARG_BYTE,
    ARG_CHAR1,
    ARG_BYTEARRAY,
    ARG_STRING,
    ARG_VOID
} argument_type_t;

typedef struct routine_argument_t {
    argument_type_t arg_type;
    size_t arg_size;
    int is_size_variadic;
} routine_argument_t;


typedef struct routine_argument_list_t {
    size_t buff_size;
    size_t buff_cap;
    size_t count;
    routine_argument_t *args;
} routine_argument_list_t;

typedef struct routine_meta_info_t {
    routine_argument_list_t argument_list;
    routine_argument_t result_type;
} routine_meta_info_t;

routine_argument_list_t routine_args(size_t count, ...);

void routine_argument_list_init(routine_argument_list_t *lst, size_t cap);

void routine_argument_list_push(routine_argument_list_t *lst, routine_argument_t argument);

void print_arg(FILE *file, routine_argument_t argument, int line_feed);

void print_routine_meta_info(FILE *file, routine_meta_info_t *meta_info);

static routine_argument_t argument_void = {.arg_type=ARG_VOID, .arg_size=0, .is_size_variadic=0};
static routine_argument_t argument_long = {.arg_type=ARG_LONG, .arg_size=sizeof(long), .is_size_variadic=0};
static routine_argument_t argument_int = {.arg_type=ARG_INT, .arg_size=sizeof(int), .is_size_variadic=0};
static routine_argument_t argument_short = {.arg_type=ARG_SHORT, .arg_size=sizeof(short), .is_size_variadic=0};
static routine_argument_t argument_byte = {.arg_type=ARG_BYTE, .arg_size=sizeof(char), .is_size_variadic=0};
static routine_argument_t argument_bytearray = {.arg_type=ARG_BYTEARRAY, .arg_size=-1, .is_size_variadic=1};
static routine_argument_t argument_char1 = {.arg_type=ARG_CHAR1, .arg_size=-1, .is_size_variadic=1};
static routine_argument_t argument_string = {.arg_type=ARG_STRING, .arg_size=-1, .is_size_variadic=1};


void interactive_routine_call(char *routine_name, int sockfd);

void write_meta_depend(routine_argument_t arg, message_t *message);

void write_long(FILE *file, long value);

void write_int(FILE *file, int value);

void write_short(FILE *file, short value);

void write_byte(FILE *file, byte_t value);

void write_char(FILE *file, char value);

void write_bytearray(FILE *file, byte_t *bytearray, int size);

void write_string(FILE *file, char *string, int size);


void read_meta_depend(routine_argument_t arg, message_t *message);

void read_long(FILE *file, long *value);

void read_int(FILE *file, int *value);

void read_short(FILE *file, short *value);

void read_byte(FILE *file, byte_t *value);

void read_char(FILE *file, char *value);

void read_bytearray(FILE *file, byte_t *bytearray, int size);

void read_string(FILE *file, char *string, int size);

#endif //REMOTEPROCEDURECALL_META_INFO_H
