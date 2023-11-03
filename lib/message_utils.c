#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include "message_utils.h"


void message_realloc_internal(message_t *msg, size_t new_size) {
    if (msg->buff_size + new_size >= msg->buff_cap) {
        msg->buff_cap += msg->buff_size + new_size;
        msg->buff_cap *= 2;
        void *new_buff = malloc(msg->buff_cap);
        memcpy(new_buff, msg->buff, msg->buff_size);
        msg->buff = new_buff;
    }
}

void message_push(message_t *msg, void *arg, int arg_size) {
    message_realloc_internal(msg, arg_size);
    memcpy(msg->buff + msg->buff_size, arg, arg_size);
    msg->buff_size += arg_size;
}

void message_init(message_t *message, size_t initial_cap) {
    bzero(message, sizeof(message_t));
    message->buff_cap = initial_cap;
    message->buff = calloc(sizeof(byte_t), initial_cap);
}

void message_pop(message_t *msg, void *dest, int size) {
    memcpy(dest, msg->buff + msg->buff_size - size, size);
    msg->buff_size -= size;
}

void message_serialize(message_t *msg, void **dest, size_t *dest_size) {
    size_t total_size = msg->buff_size + sizeof(msg->buff_size);
    if (total_size > *dest_size) {
        void *new_buff = malloc(total_size);
        memcpy(new_buff, *dest, *dest_size);
        free(*dest);
        *dest_size = total_size;
        *dest = new_buff;
    }
    memcpy(*dest, &msg->buff_size, sizeof(msg->buff_size));
    memcpy(*dest + sizeof(msg->buff_size), msg->buff, msg->buff_size);
}

message_t message_deserialize_new(void *src) {
    message_t result;
    memcpy(&result.buff_size, src, sizeof(size_t));
    result.buff_cap = result.buff_size;
    result.buff = malloc(result.buff_size);
    memcpy(result.buff, src + sizeof(size_t), result.buff_size);
    return result;
}

void message_free(message_t *message) {
    free(message->buff);
}

message_t message_deserialize_fd_new(int fd) {
    message_t result;

    bzero(&result, sizeof(result));
    ssize_t recv_status = recv(fd, &result.buff_size, sizeof(result.buff_size), 0);
    if (recv_status == -1) {
        message_init(&result, sizeof(command_t));
        message_push_cmd(&result, CM_CLOSE);
    }
    if (result.buff_size < 0) {
        return result;
    }
    result.buff_cap = result.buff_size;
    result.buff = malloc(result.buff_size);
    recv_status = recv(fd, result.buff, result.buff_size, 0);
    if (recv_status == -1) {
        message_push_cmd(&result, CM_CLOSE);
    }
    return result;
}

void message_serialize_fd(message_t *message, int fd) {
    send(fd, &message->buff_size, sizeof(message->buff_size), 0);
    send(fd, message->buff, message->buff_size, 0);
}

void message_append_message(message_t *msg_dest, message_t *msg_src) {
    message_realloc_internal(msg_dest, msg_src->buff_size + sizeof(msg_src->buff_size));
    memcpy(msg_dest->buff + msg_dest->buff_size, &msg_src->buff_size, sizeof(msg_src->buff_size));
    memcpy(msg_dest->buff + msg_dest->buff_size + sizeof(msg_src->buff_size), msg_src->buff, msg_src->buff_size);
    msg_dest->buff_size += msg_src->buff_size;
}

message_t buff_as_message(void *buff) {
    message_t result;
    memcpy(&result.buff_size, buff, sizeof(result.buff_size));
    result.buff = buff + sizeof(result.buff_size);
    result.buff_cap = 0;
    return result;
}

void message_push_cmd(message_t *message, command_t cmd) {
    message_push(message, &cmd, sizeof(cmd));
}

void message_push_byte_array(message_t *message, message_byte_array_t *byteArray) {
    message_push(message, byteArray->buffer, byteArray->buff_size);
    message_push(message, &byteArray->buff_size, sizeof(byteArray->buff_size));
}

message_byte_array_t message_pop_byte_array_new(message_t *msg) {
    message_byte_array_t result;
    message_pop(msg, &result.buff_size, sizeof(result.buff_size));
    void *offset = msg->buff + msg->buff_size - result.buff_size;
    result.buffer = malloc(result.buff_size + 1);
    memcpy(result.buffer, offset, result.buff_size);
    result.buffer[result.buff_size] = 0x0;
    msg->buff_size -= result.buff_size;
    return result;
}

command_t message_pop_command(message_t *message) {
    command_t result;
    message_pop(message, &result, sizeof(result));
    return result;
}

int message_pop_int(message_t *message) {
    int result;
    message_pop(message, &result, sizeof(result));
    return result;
}

void byte_array_free(message_byte_array_t *byte_array) {
    free(byte_array->buffer);
}

void message_push_int(message_t *message, int value) {
    message_push(message, &value, sizeof(value));
}

void message_push_status(message_t *msg, ret_status_t status) {
    message_push(msg, &status, sizeof(status));
}

ret_status_t message_pop_status(message_t *message) {
    ret_status_t result;
    message_pop(message, &result, sizeof(result));
    return result;
}

message_byte_array_t message_pop_byte_array(message_t *msg) {
    message_byte_array_t result;
    message_pop(msg, &result.buff_size, sizeof(result.buff_size));
    result.buffer = msg->buff + msg->buff_size - result.buff_size;
    msg->buff_size -= result.buff_size;
    return result;
}

void message_push_long(message_t *message, long value) {
    message_push(message, &value, sizeof(value));
}

void message_push_short(message_t *message, short value) {
    message_push(message, &value, sizeof(value));
}

void message_push_char(message_t *message, char value) {
    message_push(message, &value, sizeof(value));
}

void message_push_byte(message_t *message, byte_t value) {
    message_push(message, &value, sizeof(value));
}

long message_pop_long(message_t *message) {
    long result;
    message_pop(message, &result, sizeof(result));
    return result;
}

short message_pop_short(message_t *message) {
    short result;
    message_pop(message, &result, sizeof(result));
    return result;
}

char message_pop_char(message_t *message) {
    char result;
    message_pop(message, &result, sizeof(result));
    return result;
}

byte_t message_pop_byte(message_t *message) {
    byte_t result;
    message_pop(message, &result, sizeof(result));
    return result;
}

void message_peek_last_bytes(message_t *src, void *dest, int n, int offset) {
    memcpy(dest, src->buff + src->buff_size - offset, n);
}


