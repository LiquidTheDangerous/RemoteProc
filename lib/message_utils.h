#ifndef TOESOCK_MESSAGE_UTILS_H
#define TOESOCK_MESSAGE_UTILS_H

#include <stddef.h>

typedef unsigned char byte_t;


typedef enum command {
    CM_CLOSE,
    CM_ROUTINE_CALL,
    CM_ROUTINE_LIST,
    CM_FETCH_ROUTINE_META_INFO
} command_t;

typedef enum ret_status_t {
    RET_ROUTINE_NOT_FOUND,
    RET_OK,
} ret_status_t;

typedef struct message_byte_array_t {
    int buff_size;
    byte_t *buffer;
} message_byte_array_t;

typedef struct message_t {
    size_t buff_size;
    size_t buff_cap;
    byte_t *buff;
} message_t;

void message_init(message_t *msg, size_t initial_cap);

void message_append_message(message_t *msg_dest, message_t *msg_src);

void message_peek_last_bytes(message_t *src, void *dest, int n, int offset);

void message_push(message_t *msg, void *arg, int arg_size);

void message_push_status(message_t *msg, ret_status_t status);

void message_push_byte_array(message_t *message, message_byte_array_t *byteArray);

void message_push_cmd(message_t *message, command_t cmd);

void message_push_long(message_t *message, long value);

void message_push_int(message_t *message, int value);

void message_push_short(message_t *message, short value);

void message_push_char(message_t *message, char value);

void message_push_byte(message_t *message, byte_t value);

void message_pop(message_t *msg, void *dest, int size);

command_t message_pop_command(message_t *message);

long message_pop_long(message_t *message);

int message_pop_int(message_t *message);

short message_pop_short(message_t *message);

char message_pop_char(message_t *message);

byte_t message_pop_byte(message_t *message);

ret_status_t message_pop_status(message_t *message);


/*
 * copies last *(msg->buff + buff->size - sizeof(size_t)) bytes into new bytearray.
 * return value should be freed
 * */
message_byte_array_t message_pop_byte_array_new(message_t *msg);

/*
 * represents last *(msg->buff + buff->size - sizeof(size_t)) bytes as bytearray.
 * return value should be NOT freed
 * */
message_byte_array_t message_pop_byte_array(message_t *msg);

void message_serialize(message_t *msg_source, void **dest, size_t *dest_size);

void message_serialize_fd(message_t *message, int fd);

message_t message_deserialize_new(void *src);

message_t message_deserialize_fd_new(int fd);

message_t buff_as_message(void *buff);

void message_free(message_t *message);

void byte_array_free(message_byte_array_t *byte_array);

#endif //TOESOCK_MESSAGE_UTILS_H
