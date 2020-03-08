#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stddef.h>
#include <stdio.h>

#include "my_string.h"

typedef struct {
    // number of bytes to be allocated per field
    size_t size;
    // used for interactive operations
    const char* name;
} Column_t;

typedef struct {
    const Column_t* columns;
    size_t col_num;
    size_t row_size;
    // RW buffer
    FILE* buffer;
} Database_t;

Database_t Database_new(const char* filename, const Column_t* columns, size_t col_num) {
    size_t row_size = 0;
    for (size_t i = 0; i < col_num; ++i)
        row_size += columns[i].size + 1;
    Database_t res = { columns, col_num, row_size, fopen(filename, "r+b") };
    return res;
}

void Database_drop(Database_t* self) {
    if (self->buffer != NULL)
        fclose(self->buffer);
}

typedef struct {
    String_t line;
} RowsIter_t;

void Database_overview(Database_t* self) {
    puts("Database columns:");
    for (size_t i = 0; i < self->col_num; ++i)
        printf("name: %s, size: %zu\n", self->columns[i].name, self->columns[i].size);
    putchar('\n');
}

void Database_print(Database_t* self) {}

typedef enum { AddOk, AddFieldOverflow } AddStatus_t;

AddStatus_t Database_add(Database_t* self, StrSlice_t* vals) {
    // Check that all slices are not longer than should be
    for (size_t col_idx = 0; col_idx < self->col_num; ++col_idx)
        if (vals[col_idx].size > self->columns[col_idx].size)
            return AddFieldOverflow;

    // TODO: find row starting with `-` OR go to end
    fseek(self->buffer, 0, SEEK_END);
    fputc('+', self->buffer);
    for (size_t col_idx = 0; col_idx < self->col_num; ++col_idx) {
        StrSlice_fput(vals[col_idx], self->buffer);
        for (size_t pad = vals[col_idx].size; pad < self->columns[col_idx].size; ++pad)
            fputc(' ', self->buffer);
    }
    fputc('\n', self->buffer);
    return AddOk;
}

#endif
