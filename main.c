#include <stdio.h>

#include "database.h"
#include "handlers.h"
#include "vector.h"

#define ERR(e) fprintf(stderr, "ERROR: %s\n", e);

int main() {
    int ret_stat = 0;
    char* line = NULL;
    size_t line_size = 0;
    String_t word = String_new();

    Column_t columns[] = {
        {32, "fio"},
        {16, "department"},
        {16, "position"},
        {64, "home_address"},
        {16, "phone_number"},
        {128, "courses"}
    };
    FILE* buffer = fopen("database.txt", "r+");
    if (buffer == NULL) {
        ERR("Problem opening database file");
        ret_stat = 1;
        goto wipeout;
    }
    Database_t database = {columns, sizeof(columns) / sizeof(Column_t), buffer};
    Database_overview(&database);

    for (;;) {
        fputs("$ ", stdout);
        fflush(stdout);
        getline(&line, &line_size, stdin);
        line[strlen(line) - 1] = '\0';
        SplitWordsIter_t it = { line };
        int flow = 0;
        switch (SplitWordsIter_next(&it, &word)) {
            case IterEnd:
                puts("Use `help [cmd]` for help on specific command or in general");
                flow = 1;
                break;
            case IterSingleErr:
            case IterTotalErr:
                puts("Can't parse as valid arguments");
                puts("Use `help [cmd]` for help on specific command or in general");
                flow = 1;
                break;
            case IterOk:
                ;
        }
        if (flow == 1)
            continue;
        size_t i = 0;
        for (; i < handlers_num; ++i)
            if (strlen(handlers[i].pattern) == word.size
                && memcmp(handlers[i].pattern, word.str, word.size) == 0)
            {
                enum Flow flow = handlers[i].handler(it, &database);
                putchar('\n');
                switch (flow) {
                    case FlowExit:
                        goto wipeout;
                    default: ;
                }
                break;
            }
        if (i == handlers_num)
            puts("Unknown command. For list of commands use `help`\n");
    }

    wipeout:
    if (buffer != NULL)
        fclose(buffer);
    if (line != NULL)
        free(line);
    String_drop(&word);
    return ret_stat;
}
