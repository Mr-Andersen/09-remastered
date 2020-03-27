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
    size_t row_idx;
} RowsIter_t;

typedef struct {
    String_t line;
    size_t idx;
    StrSlice_t* values;
    bool alive;
} Row_t;

Row_t Row_new(size_t line_size, size_t values_num) {
    StrSlice_t* values = calloc(values_num, sizeof(StrSlice_t));
    ANZ(values, "Allocation failed");
    Row_t res = {
        String_reserve(line_size),
        0,
        values,
        false
    };
    return res;
}

void Row_drop(Row_t* self) {
    String_drop(&self->line);
    free(self->values);
}

RowsIter_t RowsIter_new(Database_t* database) {
    fflush(database->buffer);
    rewind(database->buffer);
    // additional 1 for \0 (added automatically by getline)
    RowsIter_t res = { database, 0 };
    return res;
}

// slices: [StrSlice; self->database->col_num]
IterRes RowsIter_next(RowsIter_t* self, Row_t* row) {
    ssize_t res;
    for (;;) {
        res = String_getline(&row->line, self->database->buffer);
        if (res == EOF)
            return IterEnd;
        row->idx = self->row_idx++;
        if (res == 1) {
            // Empty string, consists only of \n
            ERR("encountered empty row in database -- skipping");
            continue;
        }
        if (row->line.size != self->database->row_size) {
            fprintf(
                stderr,
                "ERROR: size of read line (%zu) != expected size of row (%zu)\n\"",
                row->line.size,
                self->database->row_size
            );
            StrSlice_fput(String_borrow(&row->line), stderr);
            fputs("\"\n", stderr);
            return IterSingleErr;
        }
        if (row->line.str[0] == '+') {
            row->alive = true;
            break;
        }
        if (row->line.str[0] == '-') {
            row->alive = false;
            break;
        }
        fprintf(
            stderr,
            "ERROR: line starts with wrong character: '%c'\n",
            row->line.str[0]
        );
    }
    size_t offset = 1;
    for (size_t i = 0; i < self->database->col_num; ++i) {
        row->values[i] = StrSlice_rstrip(
            String_get_slice(
                &row->line,
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

void Database_print(Database_t* self, bool filter_dead) {
    RowsIter_t it = RowsIter_new(self);
    Row_t row = Row_new(self->row_size, self->col_num);
    bool die = false;
    for (; !die;) {
        switch (RowsIter_next(&it, &row)) {
            case IterOk:
                if (filter_dead && !row.alive)
                    break;
                printf("%zu", row.idx);
                if (!filter_dead)
                    printf(" | %c", row.alive ? '+' : '-');
                // StrSlice_fput(String_borrow(&row.line), stdout);
                for (size_t i = 0; i < self->col_num; ++i) {
                    fputs(" | ", stdout);
                    StrSlice_fput(row.values[i], stdout);
                }
                putchar('\n');
                break;
            case IterSingleErr:
                // unreachable
            default:
                die = true;
        }
    }
    Row_drop(&row);
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
