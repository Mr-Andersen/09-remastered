#ifndef __MY_STRING_H__
#define __MY_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct String {
    // Pointer to owned data
    char* str;
    // Must be equal to strlen(str) (no '\0' in the end)
    size_t size;
    // Number of bytes str was malloc'd at
    // str = malloc(capacity);
    size_t capacity;
} String_t;

typedef struct StrSlice {
    const char* str;
    size_t size;
} StrSlice_t;

String_t String_new() {
    String_t res = { NULL, 0, 0 };
    return res;
}

String_t String_clone(String_t self) {
    char* new_str = (char*) malloc(self.size);
    memcpy(new_str, self.str, self.size);
    self.str = new_str;
    return self;
}

void String_drop(String_t* self) {
    free(self->str);
}

// void str_extend_with_slice(char** self, size_t* self_capacity, StrSlice_t slice) {
void String_extend_with_StrSlice(String_t* self, struct StrSlice slice) {
    // size_t self_len = strlen(*self);
    if (self->capacity < self->size + slice.size) {
        self->capacity = self->size + slice.size;
        self->str = (char*) realloc(self->str, self->capacity);
    }
    memcpy(self->str + self->size, slice.str, slice.size);
    self->size += slice.size;
}

// void str_extend_with_str(char** self, size_t* self_capacity, const char* rhs) {
void String_extend_with_str(String_t* self, const char* rhs) {
    StrSlice_t slice = { rhs, strlen(rhs) };
    String_extend_with_StrSlice(self, slice);
}

bool String_eq_str(String_t self, const char* str) {
    size_t str_size = strlen(str);
    if (str_size != self.size)
        return false;
    return memcmp(self.str, str, str_size) == 0;
}

int String_fput(String_t self, FILE* stream) {
    for (size_t i = 0; i < self.size; ++i)
        if (fputc(self.str[i], stream) == EOF)
            return EOF;
    return !EOF;
}

StrSlice_t String_as_ref(String_t* self) {
    StrSlice_t res = { self->str, self->size };
    return res;
}

ssize_t String_getline(String_t* self, FILE* buffer) {
    ssize_t res = getline(&self->str, &self->capacity, buffer);
    if (res == -1) {
        self->size = 0;
        return -1;
    }
    self->size = res;
    return res;
}

StrSlice_t StrSlice_new(const char* str, size_t size) {
    StrSlice_t res = { str, size };
    return res;
}

StrSlice_t StrSlice_from_raw(const char* str) {
    StrSlice_t res = { str, strlen(str) };
    return res;
}

void StrSlice_fput(StrSlice_t self, FILE* stream) {
    for (size_t i = 0; i < self.size; ++i)
        fputc(*self.str++, stream);
}

// void StrSlice_t_to_string(StrSlice_t self, char** res, size_t* res_capacity) {
void StrSlice_to_String(StrSlice_t self, String_t* res) {
    if (res->capacity < self.size) {
        res->str = (char*) realloc(res->str, self.size);
        res->capacity = self.size;
    }
    if (self.str != NULL)
        memcpy(res->str, self.str, self.size);
    res->size = self.size;
}

bool StrSlice_eq_str(StrSlice_t self, const char* str) {
    if (self.str == str)
        return true;
    if (self.str == NULL)
        return false;
    return memcmp(self.str, str, self.size) == 0;
}

StrSlice_t StrSlice_rstrip(StrSlice_t slice) {
    while (slice.size > 0 && slice.str[slice.size - 1] == ' ')
        --slice.size;
    return slice;
}

#endif