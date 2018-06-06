#pragma once

#include <stddef.h>

struct vector {
    size_t element_size;
    size_t stored;
    size_t capacity;
    void *array;
};

struct vector vector_init(size_t element_size);
size_t vector_length(const struct vector *v);
void vector_resize(struct vector *v, size_t size);
void vector_shrink(struct vector *v);
void vector_remove(struct vector *v, size_t pos);
void vector_empty(struct vector *v);

void* vector_get(const struct vector *v, size_t pos);

#define vector_append(V, X) _Generic((X),   \
        int: vector_append_int,             \
        default: vector_append_ptr          \
    )(V, X)

#define vector_set(V, P, X) _Generic((X), \
        int: vector_set_int,              \
        default: vector_set_ptr           \
    )(V, P, X)

void  vector_append_ptr(struct vector *v, void *val);
// Vector structure not modified, only the underlying array data.
void  vector_set_ptr(const struct vector *v, size_t pos, void *val);

// Int specialization
void vector_append_int(struct vector *v, int val);
void vector_set_int(const struct vector *v, size_t pos, int val);
