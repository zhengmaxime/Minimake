#include <stdlib.h>
#include "vector.h"

struct vec
{
    size_t size;
    size_t capacity;
    void **data;
};

struct vec *vec_init(size_t capacity)
{
    struct vec *v = calloc(1, sizeof (*v));
    void **data = calloc(capacity, sizeof (void *));
    if (!v || !data)
        return NULL;
    v->capacity = capacity;
    v->data = data;
    return v;
}

void vec_destroy(struct vec *v)
{
    free(v->data);
    free(v);
}

void vec_add(struct vec *v, void *elt)
{
    if (v->size == v->capacity)
    {
        v->capacity *= 2;
        v->data = realloc(v->data, v->capacity);
        if (!v->data)
            return;
    }
    v->data[v->size] = elt;
    v->size += 1;
}

void *vec_get(struct vec *v, size_t index)
{
    if (index >= v->size)
        return NULL;
    return v->data[index];
}

void vec_set(struct vec *v, size_t index, void *elt)
{
    if (index >= v->size)
        return;
    v->data[index] = elt;
}

inline size_t vec_size(struct vec *v)
{
    return v->size;
}
