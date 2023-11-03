#ifndef TOESOCK_REMOTE_ROUTINE_H
#define TOESOCK_REMOTE_ROUTINE_H

#include "message_utils.h"
#include "meta_info.h"
#include <pthread.h>
#include <bits/types/FILE.h>


typedef message_t(*message_processor)(message_t *message, ret_status_t *ret_status);

typedef struct routine_t {
    message_processor message_routine;
    const char *routine_name;
    routine_meta_info_t meta_info;
} routine_t;

typedef struct routine_registry_t {
    void *handle;
} routine_registry_t;

void routine_registry_init(routine_registry_t *routine_registry);

void register_routine(routine_registry_t *routine_registry, routine_t *routine);

routine_t *find_routine(routine_registry_t *routine_registry, const char *routine_name);

void routine_registry_free(routine_registry_t *routine_registry);

void print_routine_registry(FILE *dest, message_t *src);

#endif //TOESOCK_REMOTE_ROUTINE_H
