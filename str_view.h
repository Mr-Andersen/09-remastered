#ifndef __STR_VIEW_H__
#define __STR_VIEW_H__

#include <stddef.h>

#include "my_string.h"

typedef struct {
    char* str;
    size_t size;
    size_t capacity;
} StrView_t;

StrView_t StrView_new(char* str, size_t size, size_t capacity) {
    StrView_t res = { str, size, capacity };
    return res;
}

StrView_t String_get_view(String_t* self, size_t offset, size_t size, size_t capacity) {
    if (size > capacity) { FATAL("size > capacity"); }
    if (self->size < offset + capacity) {
        fprintf(
            stderr,
            "WARN: String_t out of bounds access in String_get_slice: self->size=%zu, offset=%zu, size=%zu\n",
            self->size, offset, size
        );
    }
    return StrView_new(self->str + offset, size, capacity);
}

StrSlice_t StrView_to_slice(StrView_t self) {
    return StrSlice_new(self.str, self.size);
}

#endif
