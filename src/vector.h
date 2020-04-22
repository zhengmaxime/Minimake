#pragma once

#include <stddef.h>

struct vec;

struct vec *vec_init(size_t capacity);

void vec_destroy(struct vec *v);

void vec_add(struct vec *v, void *elt);

void *vec_get(struct vec *v, size_t index);

void vec_set(struct vec *v, size_t index, void *elt);

size_t vec_size(struct vec *v);
