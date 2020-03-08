#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stddef.h>
#include <stdio.h>

typedef struct {
    // number of bytes to be allocated per field
    size_t size;
    // used for interactive operations
    const char* name;
} Column_t;

typedef struct {
    Column_t* columns;
    size_t col_num;
    // RW buffer
    FILE* buffer;
} Database_t;

void Database_overview(Database_t* self) {
    puts("Database columns:");
    for (size_t i = 0; i < self->col_num; ++i)
        printf("name: %s, size: %zu\n", self->columns[i].name, self->columns[i].size);
    putchar('\n');
}

#endif
