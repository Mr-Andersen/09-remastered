#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stddef.h>
#include <stdio.h>

#include "iterator.h"
#include "my_string.h"
#include "utils.h"

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
    size_t row_size = 1;
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
    Database_t* database;
    String_t line;
    size_t row_idx;
} RowsIter_t;

// Must use rewind(database->buffer) if want to start from the beginning
// and fflush(*) if want up-to-date state
RowsIter_t RowsIter_new(Database_t* database) {
    // additional 1 for \0 (added automatically by getline)
    RowsIter_t res = { database, String_reserve(database->row_size + 1) };
    return res;
}

void RowsIter_drop(RowsIter_t* self) {
    String_drop(&self->line);
}

// slices: [StrSlice; self->database->col_num]
IterRes RowsIter_next(RowsIter_t* self, StrSlice_t* slices, size_t* row_idx) {
    ssize_t res;
    for (;;) {
        res = String_getline(&self->line, self->database->buffer);
        if (res == EOF)
            return IterEnd;
        *row_idx = self->row_idx++;
        if (res == 1) {
            // Empty string, consists only of \n
            ERR("encountered empty row in database -- skipping");
            continue;
        }
        if (self->line.size != self->database->row_size) {
            fprintf(
                stderr,
                "ERROR: size of read line (%zu) != expected size of row (%zu)\n\"",
                self->line.size,
                self->database->row_size
            );
            StrSlice_fput(String_borrow(&self->line), stderr);
            fputs("\"\n", stderr);
            return IterSingleErr;
        }
        if (self->line.str[0] == '+')
            break;
        if (self->line.str[0] == '-')
            continue;
        fprintf(
            stderr,
            "ERROR: line starts with wrong character: '%c'\n",
            self->line.str[0]
        );
    }
    size_t offset = 1;
    for (size_t i = 0; i < self->database->col_num; ++i) {
        slices[i] = StrSlice_rstrip(
            String_get_slice(
                &self->line,
                offset,
                self->database->columns[i].size
            ),
            ' '
        );
        offset += self->database->columns[i].size + 1;
    }
    return IterOk;
}

void Database_overview(Database_t* self) {
    puts("Database columns:");
    for (size_t i = 0; i < self->col_num; ++i)
        printf("%zu: %s (%zu)\n", i, self->columns[i].name, self->columns[i].size);
    putchar('\n');
}

void Database_print(Database_t* self) {
    fflush(self->buffer);
    rewind(self->buffer);
    RowsIter_t it = RowsIter_new(self);
    StrSlice_t vals[self->col_num];
    size_t row_idx;
    bool die = false;
    for (; !die;) {
        switch (RowsIter_next(&it, vals, &row_idx)) {
            case IterOk:
                printf("%zu", row_idx);
                for (size_t i = 0; i < self->col_num; ++i) {
                    fputs(" : ", stdout);
                    StrSlice_fput(vals[i], stdout);
                }
                putchar('\n');
                break;
            case IterSingleErr:
                // unreachable
            default:
                die = true;
        }
    }
    RowsIter_drop(&it);
}

typedef enum { AddOk, AddFieldOverflow } AddStatus_t;

AddStatus_t Database_add(Database_t* self, StrSlice_t* vals, size_t* row_idx) {
    // Check that all slices are not longer than should be
    for (size_t col_idx = 0; col_idx < self->col_num; ++col_idx)
        if (vals[col_idx].size > self->columns[col_idx].size) {
            fputs("ERROR: Too long value: \"", stderr);
            StrSlice_fput(vals[col_idx], stderr);
            fprintf(stderr, "\" (#%zu)\n", col_idx);
            return AddFieldOverflow;
        }

    // TODO: find row starting with `-` OR go to end
    // yet, it just goes to end
    fseek(self->buffer, 0, SEEK_END);
    *row_idx = ftell(self->buffer) / self->row_size;
    printf("%zu\n", *row_idx);
    fputc('+', self->buffer);
    for (size_t col_idx = 0; col_idx < self->col_num; ++col_idx) {
        StrSlice_fput(vals[col_idx], self->buffer);
        for (size_t pad = vals[col_idx].size; pad < self->columns[col_idx].size + (col_idx != self->col_num - 1); ++pad)
            fputc(' ', self->buffer);
    }
    fputc('\n', self->buffer);
    return AddOk;
}

typedef enum {
    DeleteOk, DeleteAlready,
    DeleteOutOfBounds, DeleteWrongSymbol,
    DeleteIOErr
} DeleteStatus_t;

DeleteStatus_t Database_delete(Database_t* self, size_t idx) {
    if (fseek(self->buffer, 0, SEEK_END))
        return DeleteIOErr;
    long last_idx = ftell(self->buffer);
    if (last_idx == -1)
        return DeleteIOErr;
    size_t symbol_idx = idx * self->row_size;
    if (symbol_idx > last_idx)
        return DeleteOutOfBounds;
    if (fseek(self->buffer, symbol_idx, SEEK_SET))
        return DeleteIOErr;
    int symbol = fgetc(self->buffer);
    if (symbol == EOF)
        return DeleteIOErr;
    if (symbol == '-')
        return DeleteAlready;
    if (symbol != '+')
        return DeleteWrongSymbol;
    if (fseek(self->buffer, symbol_idx, SEEK_SET))
        return DeleteIOErr;
    if (fputc('-', self->buffer) == EOF)
        return DeleteIOErr;
    return DeleteOk;
}

typedef enum {
    ResurrectOk, ResurrectAlready,
    ResurrectOutOfBounds, ResurrectWrongSymbol,
    ResurrectIOErr
} ResurrectStatus_t;

ResurrectStatus_t Database_resurrect(Database_t* self, size_t idx) {
    if (fseek(self->buffer, 0, SEEK_END))
        return ResurrectIOErr;
    long last_idx = ftell(self->buffer);
    if (last_idx == -1)
        return ResurrectIOErr;
    size_t symbol_idx = idx * self->row_size;
    if (symbol_idx > last_idx)
        return ResurrectOutOfBounds;
    if (fseek(self->buffer, symbol_idx, SEEK_SET))
        return ResurrectIOErr;
    int symbol = fgetc(self->buffer);
    if (symbol == EOF)
        return ResurrectIOErr;
    if (symbol == '+')
        return ResurrectAlready;
    if (symbol != '-')
        return ResurrectWrongSymbol;
    if (fseek(self->buffer, symbol_idx, SEEK_SET))
        return ResurrectIOErr;
    if (fputc('+', self->buffer) == EOF)
        return ResurrectIOErr;
    return ResurrectOk;
}

#endif
