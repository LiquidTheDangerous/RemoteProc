#include <string.h>
#include <stdio.h>
#include "remote_routine.h"
#include "btree.h"


int comp_routine_internal(const void *a, const void *b, void *udata) {
    routine_t *first = a;
    routine_t *second = b;
    return strcmp(first->routine_name, second->routine_name);
};

void routine_registry_init(routine_registry_t *routine_registry) {
    routine_registry->handle = btree_new(sizeof(routine_t), 0, comp_routine_internal, NULL);
}

void register_routine(routine_registry_t *routine_registry, routine_t *routine) {
    btree_set((struct btree *) routine_registry->handle, routine);
}

void routine_registry_free(routine_registry_t *routine_registry) {
    btree_free(routine_registry->handle);
}

routine_t *find_routine(routine_registry_t *routine_registry, const char *routine_name) {
    return (routine_t *) btree_get(routine_registry->handle, &(struct routine_t) {.routine_name=routine_name});
}

void print_routine_registry(FILE *dest, message_t *src) {
    int count = message_pop_int(src);
    while (count--) {
        message_byte_array_t bytes = message_pop_byte_array_new(src);
        fprintf(dest, "%s\r\n", bytes.buffer);
        byte_array_free(&bytes);
    }
}
