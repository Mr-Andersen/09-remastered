#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*Destructor_t)(void*);

typedef struct {
    void* ptr;
    const size_t elem_size;
    size_t size;
    size_t capacity;
    Destructor_t destructor;
} Vector_t;

void default_vector_destructor(void* ptr) {}

Vector_t Vector_new(
    size_t elem_size,
    Destructor_t destructor
) {
    Vector_t res = { NULL, elem_size, 0, 0, destructor };
    return res;
}

Vector_t Vector_with_size_zeroed(
    size_t capacity,
    size_t elem_size,
    Destructor_t destructor
) {
    Vector_t res = {
        calloc(capacity, elem_size),
        elem_size,
        capacity,
        capacity,
        destructor
    };
    return res;
}

void Vector_drop(Vector_t* self) {
    for (size_t i = 0; i < self->size; ++i)
        self->destructor(
            &((char*) self->ptr)[self->elem_size * i]
        );
    free(self->ptr);
    self->ptr = NULL;
}

void Vector_resize(Vector_t* self, size_t capacity) {
    if (capacity <= self->capacity)
        return;
    self->ptr = realloc(self->ptr, capacity * self->elem_size);
    self->capacity = capacity;
}

void Vector_push(Vector_t* self, void* elem) {
    Vector_resize(self, self->size + 1);
    memcpy(
        &((char*) self->ptr)[self->elem_size * self->size],
        elem,
        self->elem_size
    );
    ++self->size;
}

void* Vector_index(Vector_t* self, size_t idx) {
    return &((char*) self->ptr)[self->elem_size * idx];
}

void Vector_pop(Vector_t* self) {
    self->destructor(&((char*) self->ptr)[self->elem_size * --self->size]);
}

void Vector_shrink(Vector_t* self) {
    self->ptr = realloc(self->ptr, self->size * self->elem_size);
    self->capacity = self->size;
}

typedef struct {
    Vector_t* vec;
    size_t curr_idx;
} VectorIter_t;

#endif
