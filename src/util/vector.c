#include "vector.h"

#include <assert.h>
#include <stdlib.h>

#define OFFSET_PTR(Ptr, Offset, TypeSize) \
    (((char*)(Ptr)) + (Offset) * (TypeSize))

struct vector vector_init(size_t element_size)
{
    struct vector v = {
        element_size,
        0,
        0,
        NULL
    };

    return v;
}

void vector_resize(struct vector *v, size_t size)
{
    assert(v && "invalid vector pointer");

    v->capacity = size;
    v->array = realloc(v->array, v->element_size * size);
}

size_t vector_length(const struct vector *v)
{
    assert(v && "invalid vector pointer");

    return v->stored;
}

void vector_shrink(struct vector *v)
{
    vector_resize(v, v->stored);
}

void vector_remove(struct vector *v, size_t pos)
{
    assert(v && "invalid vector pointer");
    assert(pos < v->stored);

    void *src = OFFSET_PTR(v->array, pos + 1, v->element_size);
    void *dst = OFFSET_PTR(v->array, pos, v->element_size);
    size_t len = (v->stored - pos) * v->element_size;

    __builtin_memmove(dst, src, len);
    v->stored -= 1;

    if (v->capacity / 2 > v->stored) {
        size_t size = v->capacity > 1 ? v->capacity / 2: 0;
        vector_resize(v, size);
    }
}

void vector_empty(struct vector *v)
{
    if (!v->array)
        return;

    free(v->array);
    __builtin_memset(v, 0, sizeof(*v));
}

void* vector_get(const struct vector *v, size_t pos)
{
    assert(v && "invalid vector pointer");
    assert(pos < v->stored);

    return OFFSET_PTR(v->array, pos, v->element_size);
}

void vector_append_ptr(struct vector *v, void *val)
{
    assert(v && "invalid vector pointer");

    if (v->stored == v->capacity) {
        size_t size = v->capacity ? v->capacity * 2: 2;
        vector_resize(v, size);
    }

    void *start = OFFSET_PTR(v->array, v->stored, v->element_size);
    __builtin_memcpy(start, val, v->element_size);
    v->stored += 1;
}

void  vector_set_ptr(const struct vector *v, size_t pos, void *val)
{
    assert(v && "invalid vector pointer");
    assert(pos < v->stored);

    
    void *start = OFFSET_PTR(v->array, pos, v->element_size);
    __builtin_memcpy(start, val, v->element_size);
}

/* specialized functions for int */
void vector_append_int(struct vector *v, int val)
{
    vector_append_ptr(v, &val);
}

void vector_set_int(const struct vector *v, size_t pos, int val)
{
    vector_set_ptr(v, pos, (void*)&val);
}
